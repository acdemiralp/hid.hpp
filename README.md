### hid.hpp
Single header C++23 wrapper for [libusb/hidapi](https://github.com/libusb/hidapi).

### Building the test:
- Run `bootstrap.[bat|sh]`. This will install doctest + hidapi, and create the cmake project under the `./build` directory.
- Run cmake on the `./build` directory and toggle `BUILD_TESTS`.
- Configure, generate, make.

### Using:
The cmake project exports the `hid::hid` target, hence you can:
```cmake
find_package         (hid CONFIG REQUIRED)
target_link_libraries([YOUR_PROJECT_NAME] PRIVATE hid::hid)
```
And then point the `hid_DIR` to the build or installation directory.
Alternatively, you can just copy `include/hid/hid.hpp` to your project.