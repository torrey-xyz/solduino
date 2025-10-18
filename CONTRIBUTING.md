# Contributing to Solduino

First off, thank you for considering contributing to Solduino! It's people like you that make Solduino such a great tool for the Solana and Arduino communities.

## Code of Conduct

This project adheres to a Code of Conduct that all contributors are expected to follow. Please read the [Code of Conduct](CODE_OF_CONDUCT.md) before contributing.

## How Can I Contribute?

### Reporting Bugs

Before creating bug reports, please check the issue list as you might find out that you don't need to create one. When you are creating a bug report, please include as many details as possible:

- **Use a clear and descriptive title** for the issue to identify the problem.
- **Describe the exact steps to reproduce the problem** in as many details as possible.
- **Describe the behavior you observed** after following the steps and point out what exactly is the problem with that behavior.
- **Describe the behavior you expected to see** instead and why.
- **Include screenshots and animated GIFs** if relevant.
- **Include hardware information** (ESP32 model, Arduino board, etc.).
- **Include code samples** that reproduce the issue if possible.

### Suggesting Enhancements

Enhancement suggestions are tracked as GitHub issues. When creating an enhancement suggestion, please include:

- **Use a clear and descriptive title** for the issue to identify the suggestion.
- **Provide a step-by-step description** of the suggested enhancement in as many details as possible.
- **Describe the current behavior** and explain which behavior you expected to see instead and why.
- **Explain why this enhancement would be useful** to most Solduino users.
- **List some other libraries or projects** where this enhancement exists, if applicable.

### Pull Requests

1. **Fork the repository** and create your branch from `main`.
2. **Make your changes** in a new git branch:
   ```bash
   git checkout -b feature/my-feature
   ```
3. **Follow the code style** guidelines (see below).
4. **Make sure your code compiles** without warnings or errors.
5. **Update the documentation** if you've changed functionality.
6. **Add or update tests** if applicable.
7. **Commit your changes** using a clear commit message (see below).
8. **Push to the branch**:
   ```bash
   git push origin feature/my-feature
   ```
9. **Open a Pull Request** on GitHub.

## Development Process

### Setting Up a Development Environment

1. Clone the repository:
   ```bash
   git clone https://github.com/torrey-xyz/solduino.git
   cd solduino
   ```

2. Copy the library to your Arduino libraries folder:
   ```bash
   # macOS/Linux
   cp -r . ~/Documents/Arduino/libraries/sol
   
   # Windows
   xcopy /E /I . %USERPROFILE%\Documents\Arduino\libraries\sol
   ```

3. Install dependencies:
   - Install ESP32 board support via Arduino Board Manager
   - Install ArduinoJson library via Library Manager

4. Open an example sketch in Arduino IDE and test your changes.

### Code Style Guidelines

- **Indentation**: Use 4 spaces (no tabs).
- **Naming**:
  - Classes: `PascalCase` (e.g., `RpcClient`)
  - Functions/Methods: `camelCase` (e.g., `getBalance`)
  - Constants: `UPPER_SNAKE_CASE` (e.g., `SOLDUINO_VERSION_MAJOR`)
  - Variables: `camelCase` (e.g., `publicKey`)
- **Comments**: Use `//` for single-line comments, `/* */` for multi-line.
- **Header guards**: Always use header guards (`#ifndef`, `#define`, `#endif`).
- **Memory**: Be mindful of memory constraints on embedded systems.
- **Error handling**: Always check return values and handle errors gracefully.

### Commit Messages

- Use the present tense ("Add feature" not "Added feature").
- Use the imperative mood ("Move cursor to..." not "Moves cursor to...").
- Limit the first line to 72 characters or less.
- Reference issues and pull requests liberally after the first line.
- Consider starting the commit message with an applicable emoji:
  - ✨ `:sparkles:` for new features
  - 🐛 `:bug:` for bug fixes
  - 📝 `:memo:` for documentation updates
  - ♻️ `:recycle:` for code refactoring
  - ⚡ `:zap:` for performance improvements
  - 🔒 `:lock:` for security fixes
  - 🎨 `:art:` for code style improvements

Example:
```
✨ Add getTransactionStatus method to RpcClient

This commit adds a new method to check transaction status with
improved error handling. Fixes issue #42.
```

## Project Structure

```
sol/
├── README.md              # Main documentation
├── LICENSE                # Apache 2.0 License
├── CONTRIBUTING.md        # This file
├── CODE_OF_CONDUCT.md     # Code of Conduct
├── library.properties     # Arduino library metadata
├── solduino.h/cpp         # Core SDK
├── rpc_client.h/cpp       # RPC client module
├── keypair.h/cpp          # Keypair management
├── transaction.h/cpp      # Transaction handling
├── crypto.h/cpp           # Cryptographic functions
├── serializer.h/cpp       # Serialization utilities
├── connection.h           # Connection management
└── examples/              # Example sketches
    ├── basic_rpc_demo/
    ├── transaction_demo/
    └── wallet_demo/
```

## Testing

- Test your changes on ESP32 hardware if possible.
- Test with different RPC endpoints (devnet, testnet).
- Verify memory usage doesn't significantly increase.
- Test error handling with invalid inputs.

## Documentation

- Update README.md if you add new features or change existing behavior.
- Add code comments for complex logic.
- Update API documentation in README.md for new public methods.
- Include usage examples when adding new functionality.

## Questions?

If you have questions about contributing, feel free to:

- Open an issue with the `question` label
- Email the maintainer: parvat.raj2@gmail.com

Thank you for contributing to Solduino! 🚀
