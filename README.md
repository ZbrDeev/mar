# Mar - A Simple Text Editor Written

Mar means Morocco in French (Maroc). Mar was created to offer a simple text editor that is quick to use and write, and provides basic keyboard commands. It supports UTF-8 encoding.

## Installation

To get started with Mar, follow the steps below:

### Requirements

- C Compiler (GCC or Clang)
- CMake

### Steps

1. **Clone the Repository**

```bash
git clone https://github.com/ZbrDeev/mar.git
cd mar
```

2. **Compile the Code**

```bash
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release -G Ninja ..
ninja -jN
```

## Usage

Supported commandes are in [COMMAND.md](https://github.com/ZbrDeev/mar/blob/main/COMMAND.md).

Once you have compiled, you can run mar with your file you want to modify.

```bash
./mar myfile.txt
```

## Contributing

If you would like to contribute to mar, feel free to fork the repository, make changes, and create pull requests. All contributions are welcome!

### Steps to Contribute:

1.  Fork the repository.
2.  Make your changes and commit them (`git add . && git commit -m 'Add new feature' && git push origin main`).
3.  Create a new pull request.

## License

Mar is open-source software released under the MIT License.
