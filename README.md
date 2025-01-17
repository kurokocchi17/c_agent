# C Agent 🤖

![C Agent Logo](docs/assets/C.png)

A C implementation of an AI agent framework, inspired by the classic Eliza project. C Agent provides a lightweight and efficient foundation for building AI agents with modern capabilities.

[![Twitter Follow](https://img.shields.io/twitter/follow/Kurokocchi_dev?style=social)](https://twitter.com/Kurokocchidev)

## Features

- 🛠️ Core agent framework implemented in pure C
- 💾 Memory management system for agent state
- 🔗 Extensible client interface for multiple platforms
- 📚 Simple message processing system
- 🚀 Easy to integrate with existing C/C++ projects
- 🧠 Modern AI model integration
- 🔄 Real-time communication support

## Prerequisites

- GCC or compatible C compiler
- Make build system
- Standard C libraries
- libwebsockets (for real-time communication)
- json-c (for JSON handling)
- libcurl (for HTTP requests)

## Building the Project

```bash
# Build the library
make

# Install the library (optional, requires sudo)
sudo make install
```

## Project Structure

```
ai_dancer/
├── include/          # Header files
├── src/             # Source files
├── lib/             # Built library files
├── examples/        # Example programs
├── tests/           # Test files (coming soon)
└── docs/            # Documentation (coming soon)
```

## Basic Usage

Here's a simple example of how to use the library:

```c
#include <ai_dancer.h>

int main() {
    /* Create an agent */
    Agent* agent = ai_dancer_create_agent(NULL);

    /* Create and process a message */
    Message* msg = ai_dancer_create_message("Hello!", "user", agent->id);
    ai_dancer_process_message(agent, msg);

    /* Cleanup */
    ai_dancer_destroy_message(msg);
    ai_dancer_destroy_agent(agent);

    return 0;
}
```

## Building Examples

```bash
# Build the examples
cd examples
gcc -o basic_chat basic_chat.c -lai_dancer

# Run the example
./basic_chat
```

## Memory Management

The library provides clear ownership semantics:

- All created objects (Agent, Message, Client) must be destroyed using their respective destroy functions
- String fields are copied, so the caller maintains ownership of input strings
- Memory is automatically freed when calling destroy functions

## Platform Support

Currently supported platforms:

- Discord (with real-time Gateway API)
- More platforms coming soon (Telegram, Twitter, etc.)

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

## Attribution

This project is inspired by the classic ELIZA chatbot, developed by Joseph Weizenbaum at MIT. While ELIZA pioneered early natural language processing, C Agent extends these concepts with modern AI capabilities and real-time communication features.

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## TODO

- [ ] Implement configuration file parsing
- [ ] Add memory persistence
- [ ] Create platform-specific clients (Telegram, etc.)
- [ ] Add comprehensive test suite
- [ ] Expand documentation
- [ ] Add more examples
