# Changelog

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