# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Planned
- WebSocket support for real-time subscriptions
- Certificate validation for HTTPS
- Secure key storage improvements
- Additional transaction building helpers

## [1.0.0] - 2026-03-16

### Added
- Sensor-to-chain example suite for thermistor, thermocouple, DHT22, MQ-135, and NEO-7M GPS workflows.
- Web3.js-style instruction composition primitives via `Instruction`, `AccountMeta`, and `Transaction::add()`.
- Program helper module support including `SystemProgram`, `TokenProgram`, and PDA derivation utilities.
- Custom program interaction examples and transaction transfer demos.

### Changed
- Expanded README docs with sensor dependencies, sensor demo index, and sensor suite architecture guidance.
- Updated module wiring so instruction and program helper APIs are included through `solduino.h`.

### Notes
- This release marks the first stable API baseline for Solduino on Arduino/ESP32.

## [0.1.0] - 2025-01-XX

### Added
- Initial release of Solduino
- RPC client implementation with HTTPS support
- Basic account operations (getAccountInfo, getBalance)
- Network monitoring functions (getVersion, getSlot, getHealth)
- Transaction operations (sendTransaction, getTransaction)
- Block operations (getBlock, getBlockCommitment)
- Token account operations (getTokenAccountsByOwner, getTokenSupply)
- Keypair generation and management
- Base58 encoding/decoding for Solana addresses
- Transaction signing capabilities
- Comprehensive documentation and examples
- Support for ESP32 and ESP32 variants
- Example sketches for basic usage

### Documentation
- Complete README with installation instructions
- API reference documentation
- Architecture overview
- Troubleshooting guide
- Contribution guidelines
- Code of Conduct
- Security policy

[Unreleased]: https://github.com/torrey-xyz/solduino/compare/v1.0.0...HEAD
[1.0.0]: https://github.com/torrey-xyz/solduino/releases/tag/v1.0.0
[0.1.0]: https://github.com/torrey-xyz/solduino/releases/tag/v0.1.0
