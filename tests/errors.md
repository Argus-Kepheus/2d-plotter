sketch.ino:129:36: error: no matching function for call to 'DebouncedButton::DebouncedButton(<brace-enclosed initializer list>)'
 DebouncedButton homeButton{HOME_PIN};
                                    ^
sketch.ino:99:8: note: candidate: DebouncedButton::DebouncedButton()
 struct DebouncedButton {
        ^~~~~~~~~~~~~~~
sketch.ino:99:8: note:   candidate expects 0 arguments, 1 provided
sketch.ino:99:8: note: candidate: constexpr DebouncedButton::DebouncedButton(const DebouncedButton&)
sketch.ino:99:8: note:   no known conversion for argument 1 from 'const uint8_t {aka const unsigned char}' to 'const DebouncedButton&'
sketch.ino:99:8: note: candidate: constexpr DebouncedButton::DebouncedButton(DebouncedButton&&)
sketch.ino:99:8: note:   no known conversion for argument 1 from 'const uint8_t {aka const unsigned char}' to 'DebouncedButton&&'
sketch.ino:130:36: error: no matching function for call to 'DebouncedButton::DebouncedButton(<brace-enclosed initializer list>)'
 DebouncedButton zButton{JOY_SEL_PIN};
                                    ^
sketch.ino:99:8: note: candidate: DebouncedButton::DebouncedButton()
 struct DebouncedButton {
        ^~~~~~~~~~~~~~~
sketch.ino:99:8: note:   candidate expects 0 arguments, 1 provided
sketch.ino:99:8: note: candidate: constexpr DebouncedButton::DebouncedButton(const DebouncedButton&)
sketch.ino:99:8: note:   no known conversion for argument 1 from 'const uint8_t {aka const unsigned char}' to 'const DebouncedButton&'
sketch.ino:99:8: note: candidate: constexpr DebouncedButton::DebouncedButton(DebouncedButton&&)
sketch.ino:99:8: note:   no known conversion for argument 1 from 'const uint8_t {aka const unsigned char}' to 'DebouncedButton&&'
Error during build: exit status 1