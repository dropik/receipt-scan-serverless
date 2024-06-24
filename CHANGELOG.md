# Changelog

## v1.0.2
- #1: Implemented automatic reconnecting to the database.
- #2: Better sanitising of categories.
- #3: fixed typo.
- Implemented compile-time dependency injection.
- Added integration tests for repository. Added unit tests for scanner.

## v1.0.1
- Removed Cognito User Pool from the stack. Using external User Pool.
- Improved security on attempt to access cross user resources.

## v1.0.0
- Initial release.
- Configured deployment.
- Implemented API for user, file, receipt and category.
- Implemented authentication with AWS Cognito User Pool.