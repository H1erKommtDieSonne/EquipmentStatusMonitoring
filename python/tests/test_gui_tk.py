"""
@brief Простенький smoke-тест для GUI на Tkinter.

Проверяет, что окно создаётся и корректно уничтожается.
"""

import unittest
import tkinter as tk

import gui_tk


class GuiSmokeTest(unittest.TestCase):
    """
    @brief Smoke-тесты для :class:`gui_tk.App`.
    """

    def test_app_creation_and_destroy(self) -> None:
        """
        @brief Создать и уничтожить окно приложения.

        Если Tkinter не может инициализировать дисплей (например, CI без GUI),
        тест помечается как пропущенный.
        """
        try:
            app = gui_tk.App()
        except tk.TclError:
            self.skipTest("Tkinter не может инициализировать дисплей.")
            return

        try:
            app.update_idletasks()
        finally:
            app.destroy()


if __name__ == "__main__":
    unittest.main()
