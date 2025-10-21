# Security Policy

## Supported Versions

We release patches for security vulnerabilities. Which versions are eligible for receiving such patches depends on the CVSS v3.0 Rating:

| Version | Supported          |
| ------- | ------------------ |
| 0.1.x   | :white_check_mark: |

## Reporting a Vulnerability

If you discover a security vulnerability in Solduino, please follow these steps:

1. **Do NOT** create a public GitHub issue.
2. Email the details to: **parvat.raj2@gmail.com**
3. Include the following information:
   - Description of the vulnerability
   - Steps to reproduce the issue
   - Potential impact
   - Suggested fix (if any)

### What to Expect

- **Initial Response**: We will acknowledge receipt of your report within 48 hours
- **Assessment**: We will assess and validate the vulnerability within 7 days
- **Resolution**: We will work on a fix and keep you updated on the progress
- **Disclosure**: After the fix is released, we may publicly disclose the vulnerability (with credit to you, if desired)

### Scope

Please report security vulnerabilities in:
- Cryptographic implementations
- Key management and storage
- Network communication
- Memory safety issues
- Authentication/authorization flaws

**Out of Scope:**
- Denial of service attacks
- Social engineering
- Physical attacks
- Issues requiring physical access to the device

## Security Best Practices

When using Solduino in production:

1. **Private Key Security**: Never hardcode private keys. Use secure storage solutions.
2. **Certificate Validation**: Implement proper SSL/TLS certificate validation for production.
3. **Network Security**: Always use HTTPS endpoints. Consider using private RPC endpoints.
4. **Hardware Security**: Consider using hardware security modules (HSM) for critical applications.
5. **Regular Updates**: Keep the library updated to the latest version.
6. **Secure Boot**: Enable secure boot features on ESP32 when available.

## Known Security Considerations

- The library currently uses `setInsecure()` for HTTPS connections, which disables certificate validation. This is acceptable for development but **should be replaced with proper certificate validation in production**.
- Embedded systems have limited resources, which may impact security features. Always assess your threat model.

Thank you for helping keep Solduino and its users safe!
