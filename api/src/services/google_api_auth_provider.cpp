//
// Created by Daniil Ryzhkov on 19/10/2024.
//

#include "google_api_auth_provider.hpp"
#include "../dependencies/jwt-cpp/jwt.h"
#include "lambda/log.hpp"
#include <aws/core/utils/json/JsonSerializer.h>
#include <aws/core/utils/base64/Base64.h>

using namespace api::services;
using namespace Aws::Utils::Json;
using namespace Aws::Utils::Base64;
using namespace Aws::Http;

class json_traits_facade {
 public:
  json_traits_facade() = default;

  explicit json_traits_facade(const JsonValue &val) {
    m_wrappee = val;
  }

  explicit json_traits_facade(const JsonView &view) {
    m_wrappee = view.Materialize();
  }

  explicit json_traits_facade(const std::map<std::string, json_traits_facade> &map) {
    for (const auto &pair : map) {
      m_wrappee.WithObject(pair.first, pair.second.m_wrappee);
    }
  }

  explicit json_traits_facade(const std::vector<json_traits_facade> &vec) {
    JsonValue valWithArray;
    std::vector<JsonValue> json_values;
    json_values.reserve(vec.size());
    for (const auto &item : vec) {
      json_values.push_back(item.m_wrappee);
    }

    Aws::Utils::Array<JsonValue> arr(json_values.data(), json_values.size());
    valWithArray.WithArray("arr", arr);
    m_wrappee = valWithArray.View().GetObject("arr").Materialize();
  }

  explicit json_traits_facade(const std::string &str) {
    m_wrappee = JsonValue(str);
  }

  explicit json_traits_facade(const char *str) {
    m_wrappee = JsonValue(std::string{str});
  }

  explicit json_traits_facade(double num) {
    JsonValue val;
    val.WithDouble("val", num);
    m_wrappee = val.View().GetObject("val").Materialize();
  }

  explicit json_traits_facade(int64_t num) {
    JsonValue val;
    val.WithInt64("val", num);
    m_wrappee = val.View().GetObject("val").Materialize();
  }

  explicit json_traits_facade(bool b) {
    JsonValue val;
    val.WithBool("val", b);
    m_wrappee = val.View().GetObject("val").Materialize();
  }

  [[nodiscard]] JsonValue get() const {
    return m_wrappee;
  }

 private:
  JsonValue m_wrappee;
};

struct traits {
  // Type Specifications
  using value_type = json_traits_facade; // The generic "value type" implementation, most libraries have one
  using object_type = std::map<std::string, json_traits_facade>; // The "map type" string to value
  using array_type = std::vector<json_traits_facade>; // The "list type" array of values
  using string_type = std::string; // The "list of chars", must be a narrow char
  using number_type = double; // The "precision type"
  using integer_type = int64_t; // The "integral type"
  using boolean_type = bool; // The "boolean type"

  // Translation between the implementation notion of type, to the jwt::json::type equivalent
  static jwt::json::type get_type(const value_type &val) {
    using jwt::json::type;

    auto view = val.get().View();

    if (view.IsObject())
      return type::object;
    if (view.IsListType())
      return type::array;
    if (view.IsString())
      return type::string;
    if (view.IsFloatingPointType())
      return type::number;
    if (view.IsIntegerType())
      return type::integer;
    if (view.IsBool())
      return type::boolean;

    throw std::logic_error("invalid type");
  }

  // Conversion from generic value to specific type
  static object_type as_object(const value_type &val) {
    object_type obj;
    auto objects = val.get().View().GetAllObjects();
    for (const auto &member : objects) {
      obj[member.first] = json_traits_facade(member.second);
    }
    return obj;
  }

  static array_type as_array(const value_type &val) {
    array_type arr;
    auto objects = val.get().View().GetAllObjects();
    for (const auto &element : objects) {
      arr.emplace_back(element.second);
    }
    return arr;
  }

  static string_type as_string(const value_type &val) {
    return val.get().View().AsString();
  }

  static number_type as_number(const value_type &val) {
    return val.get().View().AsDouble();
  }

  static integer_type as_integer(const value_type &val) {
    return val.get().View().AsInt64();
  }

  static boolean_type as_boolean(const value_type &val) {
    return val.get().View().AsBool();
  }

  // serialization and parsing
  static bool parse(value_type &val, string_type str) {
    JsonValue json(str);
    if (!json.WasParseSuccessful())
      return false;
    val = json_traits_facade(json);
    return true;
  }

  // with no extra whitespace, padding or indentation
  static string_type serialize(const value_type &val) {
    return val.get().View().WriteCompact();
  }
};

std::string base64_encode(const std::string &input) {
  Base64 base64;
  Aws::Utils::Array<unsigned char> buf((unsigned char*)input.c_str(), input.size());
  return base64.Encode(buf);
}

struct google_api_auth_result {
  std::string access_token;

  JSON_BEGIN_SERIALIZER(google_api_auth_result)
      JSON_PROPERTY("access_token", access_token)
  JSON_END_SERIALIZER()
};

std::string auth_url = "https://oauth2.googleapis.com/token";

google_api_auth_provider::google_api_auth_provider(std::shared_ptr<api::settings::google_api_settings> settings)
    : m_settings(std::move(settings)) {}

std::string google_api_auth_provider::get_access_token() {
  if (m_access_token.has_value()) {
    return m_access_token.value();
  }

  auto token = jwt::create<traits>()
      .set_header_claim("kid", json_traits_facade(m_settings->private_key_id))
      .set_issuer(m_settings->client_email)
      .set_payload_claim("scope", json_traits_facade("https://www.googleapis.com/auth/androidpublisher"))
      .set_audience(auth_url)
      .set_issued_now()
      .set_expires_in(std::chrono::seconds{60})
      .sign(jwt::algorithm::rs256("", m_settings->private_key, "", ""), base64_encode);

  std::string body = "grant_type=urn:ietf:params:oauth:grant-type:jwt-bearer&assertion=" + token;
  auto outcome = m_client.post<google_api_auth_result>(auth_url, body, content_type::form);
  if (outcome.is_success) {
    m_access_token = outcome.result.access_token;
    return m_access_token.value();
  } else {
    lambda::log.error("Failed to get access token. Google API responded with %d: %s.",
                      outcome.status_code,
                      outcome.error.value_or(""));
    throw std::runtime_error("Failed to get access token.");
  }
}
