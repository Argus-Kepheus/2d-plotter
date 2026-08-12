"""Static consistency checks for the 2D Plotter Wokwi project."""

from __future__ import annotations

import json
import re
import unittest
from collections import Counter
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
DIAGRAM_PATH = ROOT / "diagram.json"
SKETCH_PATH = ROOT / "sketch.ino"
LIBRARIES_PATH = ROOT / "libraries.txt"


class ProjectConsistencyTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.diagram = json.loads(DIAGRAM_PATH.read_text(encoding="utf-8"))
        cls.sketch = SKETCH_PATH.read_text(encoding="utf-8")
        cls.libraries = {
            line.strip()
            for line in LIBRARIES_PATH.read_text(encoding="utf-8").splitlines()
            if line.strip() and not line.lstrip().startswith("#")
        }
        cls.parts_by_id = {part["id"]: part for part in cls.diagram["parts"]}
        cls.connections = {
            (connection[0], connection[1])
            for connection in cls.diagram["connections"]
        }

    def test_diagram_shape_and_counts(self) -> None:
        self.assertIsInstance(self.diagram.get("parts"), list)
        self.assertIsInstance(self.diagram.get("connections"), list)
        self.assertEqual(31, len(self.diagram["parts"]))
        self.assertEqual(113, len(self.diagram["connections"]))

    def test_part_ids_are_unique(self) -> None:
        ids = [part["id"] for part in self.diagram["parts"]]
        self.assertEqual(len(ids), len(set(ids)))

    def test_required_component_counts(self) -> None:
        counts = Counter(part["type"] for part in self.diagram["parts"])
        expected = {
            "wokwi-arduino-uno": 1,
            "wokwi-stepper-motor": 2,
            "wokwi-a4988": 2,
            "wokwi-74hc595": 3,
            "wokwi-led-bar-graph": 2,
            "wokwi-analog-joystick": 1,
            "wokwi-servo": 1,
            "board-ssd1306": 1,
            "wokwi-pushbutton": 2,
            "wokwi-relay-module": 2,
        }
        for part_type, expected_count in expected.items():
            self.assertEqual(expected_count, counts[part_type], part_type)

    def test_all_connection_endpoints_reference_known_parts(self) -> None:
        known_ids = set(self.parts_by_id)
        for connection in self.diagram["connections"]:
            for endpoint in connection[:2]:
                part_id, separator, pin = endpoint.partition(":")
                self.assertEqual(":", separator, endpoint)
                self.assertTrue(pin, endpoint)
                self.assertIn(part_id, known_ids, endpoint)

    def test_critical_wiring(self) -> None:
        required = {
            ("uno:2", "drvX:STEP"),
            ("uno:3", "drvX:DIR"),
            ("uno:4", "drvY:STEP"),
            ("uno:5", "drvY:DIR"),
            ("uno:6", "sr0:DS"),
            ("uno:7", "sr0:SHCP"),
            ("uno:8", "sr0:STCP"),
            ("uno:9", "servoZ:PWM"),
            ("joy1:SEL", "uno:10"),
            ("btnHome:2.l", "uno:11"),
            ("btnEStop:2.l", "uno:12"),
            ("uno:13", "drvX:ENABLE"),
            ("uno:13", "drvY:ENABLE"),
            ("joy1:HORZ", "uno:A0"),
            ("joy1:VERT", "uno:A1"),
            ("uno:A2", "relaySuction:IN"),
            ("uno:A3", "relayFan:IN"),
            ("oled1:SDA", "uno:A4"),
            ("oled1:SCL", "uno:A5"),
            ("sr0:Q7S", "sr1:DS"),
            ("sr1:Q7S", "sr2:DS"),
        }
        self.assertEqual(set(), required - self.connections)

    def test_firmware_pin_constants(self) -> None:
        expected = {
            "X_STEP_PIN": "2",
            "X_DIR_PIN": "3",
            "Y_STEP_PIN": "4",
            "Y_DIR_PIN": "5",
            "BAR_DATA_PIN": "6",
            "BAR_CLOCK_PIN": "7",
            "BAR_LATCH_PIN": "8",
            "SERVO_Z_PIN": "9",
            "JOY_SEL_PIN": "10",
            "HOME_PIN": "11",
            "ESTOP_PIN": "12",
            "STEPPERS_ENABLE_PIN": "13",
            "JOY_X_PIN": "A0",
            "JOY_Y_PIN": "A1",
            "RELAY_SUCTION_PIN": "A2",
            "RELAY_FAN_PIN": "A3",
        }
        found = dict(
            re.findall(
                r"constexpr\s+uint8_t\s+(\w+_PIN)\s*=\s*(A\d+|\d+)\s*;",
                self.sketch,
            )
        )
        for name, value in expected.items():
            self.assertEqual(value, found.get(name), name)

    def test_declared_libraries_cover_includes(self) -> None:
        expected = {
            "AccelStepper",
            "Adafruit GFX Library",
            "Adafruit SSD1306",
            "Servo",
        }
        self.assertEqual(expected, self.libraries)
        for header in (
            "AccelStepper.h",
            "Adafruit_GFX.h",
            "Adafruit_SSD1306.h",
            "Servo.h",
        ):
            self.assertIn(f"#include <{header}>", self.sketch)

    def test_loop_is_non_blocking_and_prioritizes_estop(self) -> None:
        uncommented = re.sub(r"//.*", "", self.sketch)
        self.assertNotRegex(uncommented, r"\bdelay\s*\(")
        estop_check = self.sketch.index("if (!emergency && digitalRead(ESTOP_PIN)")
        home_check = self.sketch.index("if (homeButton.fell()")
        self.assertLess(estop_check, home_check)
        self.assertIn("digitalWrite(STEPPERS_ENABLE_PIN, HIGH);", self.sketch)
        self.assertRegex(
            self.sketch,
            r"if \(emergency\) \{[\s\S]*?return;[\s\S]*?\}",
        )


if __name__ == "__main__":
    unittest.main()
