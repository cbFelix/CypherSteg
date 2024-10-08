# CypherSteg

CypherSteg is a project that provides tools for hiding and extracting encrypted data from PNG files using steganography. The tools included allow users to hide sensitive information within images and retrieve it later, ensuring privacy and security.

## Features

- **Encrypt and hide data in PNG files** using the `CypherEncrypt` tool.
- **Decrypt and extract hidden data** from PNG files using the `CypherDecrypt` tool.
- **Cross-platform** support, recommended for use on Linux systems.
- Precompiled binaries are available for **quick usage**.

## Disclaimer

This project is intended for **educational and research purposes only**. Unauthorized use of encryption and steganography may be illegal in certain jurisdictions. It is the user’s responsibility to ensure compliance with local laws regarding data encryption, privacy, and security.

The authors of this project are not responsible for any misuse or damages caused by the use of this software. **Use at your own risk.**

## Installation

### Cloning the Repository

To get started, clone the repository:

```bash
git clone https://github.com/cbFelix/CypherSteg.git
cd CypherSteg
```

## Compiling from Source

For users who want to compile the code themselves, the source files are provided in the `src` directory.

You will need to have gcc and libpng installed on your system to compile the code.

## Compile the Encryption Tool:

```bash
cd src
gcc -o CypherEncrypt cypherencrypt.c -lpng
```

## Compile the Decryption Tool:
```bash
gcc -o CypherDecrypt cypherdecrypt.c -lpng
```

The resulting binaries will be named `CypherEncrypt` and `CypherDecrypt` respectively.

# Precompiled Binaries

For convenience, precompiled binaries are available in the release tab of the same repository. These builds include:

- `CypherEncrypt`: a tool for encrypting and hiding data in PNG files.
- `CypherDecrypt`: a tool for decrypting and extracting data from PNG files.
- `License`
- `README.md file`

To use the precompiled binaries, simply go to the release tab, download the precompiled build from there, and run the programs you need in their:

```bash
sudo ./CypherEncrypt <png_image_path> <file>
sudo ./CypherDecrypt <enctypted_image>
```

# Usage
## Encrypting Data

To encrypt and hide data in a PNG file, use the `CypherEncrypt` tool:

```bash
./CypherEncrypt <path_to_png> <path_to_file_to_hide>
```
- `<path_to_png>`: The path to the PNG file where the data will be hidden.
- `<path_to_file_to_hide>`: The path to the file you want to encrypt and hide.


## Decrypting Data

To extract hidden data from a PNG file, use the CypherDecrypt tool:

```bash
./CypherDecrypt <path_to_png>
```

- `<path_to_png>`: The path to the PNG file that contains hidden data.

The decrypted file will be saved in the `output/decrypt` folder.

## Dependencies

To compile and run the CypherSteg tools, the following dependencies are required:

- libpng: Library used for handling PNG files.
    - Install on Ubuntu/Debian:
    ```bash
    sudo apt install libpng-dev
    ```

## Project Structure

- `src/` – Source code for the encryption and decryption tools (`cypherencrypt.c` and `cypherdecrypt.c`).
- `README.md` – Documentation file.
- `LICENSE` – MIT License file.


