## JSON

A small C++20 library that can parse and serialize JSON.

### Building

You must have `premake5` and a supported build system.

Builds can be found inside the `build` folder, and include files inside the `include` directory and `libs` folder.

See the `json-dev` project inside [`premake5.lua`](https://github.com/Marco4413/json/blob/master/premake5.lua#L13) to understand what include paths are needed for the `json` project.

### Running Tests

Build the `json-dev` project and run its executable. The executable's working directory **MUST** be the root of this repo.

### Resources

- JSON spec:
  - [https://www.json.org/json-en.html](https://www.json.org/json-en.html)
  - [https://www.ecma-international.org/publications-and-standards/standards/ecma-404/](https://www.ecma-international.org/publications-and-standards/standards/ecma-404/)
- UTF-8: [https://en.wikipedia.org/wiki/UTF-8](https://en.wikipedia.org/wiki/UTF-8)
- Test Files: [https://github.com/nst/JSONTestSuite](https://github.com/nst/JSONTestSuite)
