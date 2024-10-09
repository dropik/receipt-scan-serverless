# Changelog

## v1.1.6
- Extended rest API library to support `DELETE` method without capturing any parameters.
- Added `DELETE` method to delete user. By deleting user, all user's receipts, categories and budgets are deleted. Also all user's files are deleted from S3 bucket. And finally, user is deleted from Cognito User Pool.
- Added CORS support to all API endpoints. Supported endpoints are `https://speza.it` and `http://localhost:5173`.

## v1.1.5
- Added icon field to category.

## v1.1.4
- Deleting all receipt items, when updating a receipt.
- Using transactions when storing receipt.

## v1.1.3
- When receipt contains items with same categories, the response contains list of categories with unique names.

## v1.1.2
- Setting timezone to UTC once connection to database is established. This ensures that all timestamps comparisons are done in UTC. Client should always send timestamps in UTC.

## v1.1.1
- Updating entity version is now responsibility of the client. Therefore, re-updating entity with same version yields to concurrency exception. 

## v1.1.0
- Migrated to Amazon Linux 2023 runtime.
- Migrated to C++23 standard.
- Reorganized CMake. Better dependency management. Made build compatible with LLVM and CLang.
- Implemented budgets endpoints.
- Better separation between service layer and repository layer.
- Implemented optimistic concurrency handling on updating entities.
- Implemented endpoints to obtain list of changes for budgets, categories and receipts.
- Only one file per receipt is allowed. Scanning assumes only one document from image. Hence, images with multiple receipts in single image are not supported.
- The new flow to start receipt image scanning is to create first the receipt entity, and then call the upload image endpoint for the created receipt.
- Removed any seeding. All seeding is assumed to be done from the client and is the responsibility of the client.
- Implemented integration tests for scanner and api lambdas.

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