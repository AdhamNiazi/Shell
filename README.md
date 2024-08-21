
# SimpleShell

SimpleShell is a custom-built command-line shell designed for basic and advanced command execution. This lightweight shell offers essential features to handle various command-line operations efficiently, making it an ideal project for those interested in understanding the inner workings of shell environments.

## Key Features:
- **Background Task Execution:** Supports running commands in the background using the `&` operator. Handles errors such as mislocated background signs and multiple instances of `&`.
- **Output Redirection:** Implements both appending (`>>`) and truncating (`>`) output redirection. Includes error handling for missing commands before redirection, missing output files, and mislocated output redirection operators.
- **Pipe Handling:** Allows for the use of pipes (`|`) to connect multiple commands. Includes validation to ensure commands are provided before and after the pipe, and handles error cases like mislocated pipe characters.
- **Forking and Process Management:** Utilizes forking to create new processes, enabling concurrent command execution and process management.
- **Command Parsing:** Efficiently tokenizes and parses command inputs, checking for syntax errors and ensuring correct execution order.
- **Basic Command Support:** Includes essential commands such as pwd to print the current working directory, and echo to display a line of text or variables. These basic commands are implemented with robust error handling to ensure smooth operation within the shell environment.
## Usage:
SimpleShell is designed to be straightforward, yet powerful enough to handle typical shell tasks. Whether you’re redirecting output, running background processes, or chaining commands with pipes, SimpleShell provides a robust environment for your command-line operations.

## Getting Started:
Clone the repository and compile the source code to start using SimpleShell. Explore the various features and customize the shell to suit your needs.
