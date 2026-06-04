![Project Banner](./docs/screenshots/banner.png)

TruePass is a secure, zero-knowledge password manager built on a client/server architecture. It features end-to-end encryption (E2E) and uses the OPAQUE augmented PAKE protocol for secure authentication, ensuring the server never handles or stores user master passwords.

## Features

- **End-to-End Encrypted**: All sensitive data is encrypted on the client side before synchronization.
- **Zero-Knowledge Model**: The server has no access to user passwords or decrypted vault contents.
- **OPAQUE (Augmented PAKE)**: Secure registration and login flow where the server never sees the plaintext master password.
- **Cross-Platform Client**: Built with Qt 6, featuring platform-specific TPM integration.
- **Device-Bound Auth**: Support for hardware-backed authentication.
- **Comprehensive Vault Management**: Support for logins, cards, TOTP, and secure password generation.
- **Robust Cryptography**: Shared library providing Argon2, AES-GCM, and curve-based utilities.

## Architecture

### common

The shared library contains:

- cryptographic primitives and wrappers
- curve and key management utilities
- hash, HMAC, HKDF, Argon2, and KMAC helpers
- binary encoding helpers
- UUID utilities
- validation logic
- DTOs used by both client and server

### server

The backend exposes authentication and vault-related endpoints through Drogon. It handles:

- OPAQUE registration and login flows
- session handling
- device-bound login validation
- user, vault, and item data access

### client

The client is a Qt desktop application that provides:

- login and registration UI
- vault browsing and editing
- item creation and viewing
- password generation
- secure secret handling
- TPM provider abstraction for supported platforms

## Tech Stack

- **Languages/Frameworks**: C++23, Qt 6, Drogon, CMake 3.22+
- **Cryptography**: [OpenSSL](https://www.openssl.org/), [opaque-ke](https://github.com/facebook/opaque-ke) (via [opaquepp](https://github.com/ikea400/opaquepp))
- **Serialization/Utilities**: [glaze](https://github.com/stephenberry/glaze), [ctre](https://github.com/hanickadot/compile-time-regular-expressions), [JsonCpp](https://github.com/open-source-parsers/jsoncpp)
- **UI/Graphics**: [ThorVG](https://github.com/thorvg/thorvg)
- **Testing**: [GoogleTest](https://github.com/google/googletest)

## Prerequisites

Install the required build and runtime dependencies (compiler, CMake, Qt 6, OpenSSL, etc.). If using `vcpkg`, install the matching packages or use the provided configuration.

## Build

From the repository root:

```bash
cmake -S . -B build
cmake --build build
```

## Run

### Server

The server loads its configuration from `config.json` and starts the HTTP service.

### Client

The client starts a Qt application and connects to the server. Default endpoint:

```text
https://localhost:443
```

Make sure the server is running before launching the client.

## Tests

Run the test suite from the build directory:

```bash
ctest --test-dir build
```

## Screenshots

![alt text](./docs/screenshots/login.png)

![Main page image](./docs/screenshots/main-page.png)

## Project Structure

```text
TruePass/
├── common/       # Shared crypto, DTOs, validation, utilities
├── server/       # Backend API
├── client/       # Qt desktop client
└── CMakeLists.txt
```

## Notes

- The client and server share data contracts through the common library.
- Sensitive data is handled through dedicated wrappers and secure abstractions.
- Platform-specific TPM providers are used where available.
- The server and client depend on external libraries that must be installed separately.

## License

This project is licensed under the MIT License. See [LICENSE.txt](./LICENSE.txt) for more
