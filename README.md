<p align="center">
  <img src="./docs/screenshots/banner.png" alt="TruePass Banner" width="100%">
</p>

<h1 align="center">TruePass</h1>

<p align="center">
  <em>A secure, zero-knowledge password manager built with Qt and C++23.</em>
</p>

<p align="center">
  <img alt="C++23" src="https://img.shields.io/badge/C%2B%2B-23-blue.svg?style=flat-square">
  <img alt="Qt 6" src="https://img.shields.io/badge/Qt-6.0%2B-41CD52.svg?style=flat-square">
  <img alt="License" src="https://img.shields.io/badge/license-MIT-green.svg?style=flat-square">
</p>

<br>

**TruePass** is a modern, privacy-first password manager featuring end-to-end encryption (E2E) and the OPAQUE augmented PAKE protocol. Built on a robust client-server architecture, it guarantees that the server never handles or stores your master password in plaintext, achieving a true zero-knowledge model.

---

## ✨ Key Features

- **End-to-End Encrypted (E2E):** All sensitive data is encrypted directly on the client side before any synchronization occurs.
- **Zero-Knowledge Architecture:** The server operates completely blindly, with no access to user passwords or decrypted vault contents.
- **OPAQUE (Augmented PAKE):** Utilizes a secure, hardware-backed registration and login flow ensuring your master password never leaves your device.
- **Cross-Platform Client:** A beautiful, responsive Qt 6 desktop application equipped with platform-specific TPM (Trusted Platform Module) integration.
- **Device-Bound Authentication:** Native support for hardware-backed security modules.
- **Comprehensive Vault Management:** Manage and organize your digital life with dedicated support for **Logins, Credit Cards, Identities, Secure Notes, TOTP**, and an integrated secure password generator.
- **Robust Cryptography:** Powered by a shared library offering Argon2, AES-GCM, and advanced curve-based cryptographic utilities.

---

## 🏗️ Architecture Overview

TruePass is structured into three primary components to ensure clean separation of concerns:

### `common/` (Shared Library)
The backbone of TruePass, shared between the client and server.
* **Cryptography:** Primitives and wrappers including hash, HMAC, HKDF, Argon2, and KMAC helpers.
* **Utilities:** Curve and key management, binary encoding helpers, UUID utilities, and validation logic.
* **Data Transfer:** DTOs (Data Transfer Objects) establishing the contract between client and server.

### `server/` (Backend API)
A high-performance C++ backend powered by [Drogon](https://github.com/drogonframework/drogon).
* Handles OPAQUE registration and login flows.
* Manages secure sessions and device-bound login validation.
* Safely orchestrates user, vault, and encrypted item data access.

### `client/` (Desktop Application)
A seamless Qt 6 desktop experience.
* **UI/UX:** Intuitive login, registration, and vault browsing interfaces.
* **Vault Actions:** Item creation, viewing, editing, and built-in password generation.
* **Security:** Secure secret handling and TPM provider abstraction for supported operating systems.

---

## 🛠️ Technology Stack

| Category | Technologies Used |
|---|---|
| **Languages & Frameworks** | C++23, Qt 6, Drogon, CMake 3.22+ |
| **Cryptography** | [OpenSSL](https://www.openssl.org/), [opaque-ke](https://github.com/facebook/opaque-ke) (via [opaquepp](https://github.com/ikea400/opaquepp)) |
| **Serialization & Utils** | [glaze](https://github.com/stephenberry/glaze), [ctre](https://github.com/hanickadot/compile-time-regular-expressions), [JsonCpp](https://github.com/open-source-parsers/jsoncpp) |
| **UI & Graphics** | [ThorVG](https://github.com/thorvg/thorvg) |
| **Testing** | [GoogleTest](https://github.com/google/googletest) |

---

## 🚀 Getting Started

### Prerequisites
Ensure you have the required build and runtime dependencies installed on your system:
* A modern C++23 compatible compiler (GCC, Clang, or MSVC)
* CMake (3.22 or higher)
* Qt 6
* OpenSSL
* *Note: If using `vcpkg`, install the matching packages or use the provided configuration.*

### Build Instructions
From the repository root, configure and build the project using CMake:

```bash
# Configure the project
cmake -S . -B build

# Build the project
cmake --build build
```

### Running the Application

**1. Start the Server**  
The backend server loads its configuration from `config.json` and starts the HTTP service. Make sure the server is running before launching the client.

**2. Start the Client**  
Launch the TruePass Qt application. By default, it connects to the local server endpoint:
```text
https://localhost:443
```

---

## 🧪 Testing

TruePass includes a comprehensive test suite. To run the tests from the build directory:

```bash
ctest --test-dir build --output-on-failure
```

---

## 📸 Screenshots

| Login Interface | Main Vault Dashboard |
| :---: | :---: |
| <img src="./docs/screenshots/login.png" alt="TruePass Login Screen" width="400"/> | <img src="./docs/screenshots/main-page.png" alt="TruePass Main Vault View" width="400"/> |

---

## 📜 License

This project is licensed under the MIT License. See the [LICENSE.txt](./LICENSE.txt) file for more details.
