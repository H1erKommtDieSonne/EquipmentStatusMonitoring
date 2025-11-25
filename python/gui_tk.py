"""
@brief GUI-приложение на Tkinter для работы с ESM через модуль esm_py

@details
    Приложение позволяет:
      * открыть/создать SQLite-базу;
      * применять миграции через Engine.migrate();
      * добавлять/просматривать работников;
      * добавлять/просматривать устройства;
      * создавать заявки (breakdown) и смотреть список задач.
"""

from __future__ import annotations

import tkinter as tk
from tkinter import ttk, messagebox

import esm_py




PRIORITY_OPTIONS = [
    ("Нет", "none", getattr(esm_py.ServicePriority, 'None')),
    ("Низкий", "low", getattr(esm_py.ServicePriority, 'Low')),
    ("Высокий", "high", getattr(esm_py.ServicePriority, 'High')),
]


PRIORITY_CODE_TO_ENUM = {code: enum for (_label, code, enum) in PRIORITY_OPTIONS}


PRIORITY_ENUM_TO_LABEL = {enum: label for (label, _code, enum) in PRIORITY_OPTIONS}


class App(tk.Tk):
    """
    @brief Главное окно приложения мониторинга оборудования.

    @details
        Содержит:
          * поле выбора БД и кнопку миграции;
          * панель устройств;
          * панель работников;
          * панель задач.
    """

    def __init__(self) -> None:
        """@brief Инициализирует окно и внутренние переменные."""
        super().__init__()
        self.title("Equipment Status Monitoring")
        self.geometry("1100x700")

        #: @brief Текущий фасад ESM (Engine) или None.
        self.engine: esm_py.Engine | None = None

        # --- Переменные формы БД ---
        self.db_path_var = tk.StringVar(value="esm.db")

        # --- Переменные формы устройств ---
        self.dev_name_var = tk.StringVar()
        self.dev_ipv4_var = tk.StringVar()
        self.dev_priority_code_var = tk.StringVar(value="high")
        self.dev_is_faulty_var = tk.BooleanVar(value=False)
        self.dev_is_reserve_var = tk.BooleanVar(value=False)
        self.dev_uptime_var = tk.StringVar()
        self.dev_standby_var = tk.StringVar()
        self.dev_fault_var = tk.StringVar()

        # --- Переменные формы работников ---
        self.worker_name_var = tk.StringVar()
        self.worker_max_jobs_var = tk.StringVar(value="1")
        self.worker_skill_var = tk.StringVar()

        # --- Переменные формы задач ---
        self.breakdown_ipv4_var = tk.StringVar()
        self.breakdown_fault_var = tk.StringVar()

        self._build_ui()



    def _build_ui(self) -> None:
        """@brief Построить все виджеты главного окна."""
        self.columnconfigure(0, weight=1)
        self.rowconfigure(0, weight=1)

        root = ttk.Frame(self, padding=8)
        root.grid(row=0, column=0, sticky="nsew")
        for col in range(2):
            root.columnconfigure(col, weight=1)
        root.rowconfigure(2, weight=1)

        self._build_db_frame(root)
        self._build_devices_frame(root)
        self._build_workers_frame(root)
        self._build_jobs_frame(root)

    def _build_db_frame(self, parent: ttk.Frame) -> None:
        """@brief Создать панель выбора файла БД."""
        frame = ttk.LabelFrame(parent, text="База данных")
        frame.grid(row=0, column=0, columnspan=2, sticky="ew", pady=(0, 8))
        frame.columnconfigure(1, weight=1)

        ttk.Label(frame, text="Файл SQLite:").grid(row=0, column=0, sticky="w")
        ttk.Entry(frame, textvariable=self.db_path_var).grid(
            row=0, column=1, sticky="ew", padx=4
        )
        ttk.Button(
            frame, text="Открыть / мигрировать", command=self.on_open_db
        ).grid(row=0, column=2, padx=4)

    def _build_devices_frame(self, parent: ttk.Frame) -> None:
        """@brief Создать панель работы с устройствами."""
        frame = ttk.LabelFrame(parent, text="Устройства")
        frame.grid(row=1, column=0, sticky="nsew", padx=(0, 4))
        frame.columnconfigure(0, weight=1)
        frame.rowconfigure(1, weight=1)

        form = ttk.Frame(frame)
        form.grid(row=0, column=0, sticky="ew", pady=(0, 4))
        for i in range(4):
            form.columnconfigure(i, weight=1)

        # --- строка 0 ---
        ttk.Label(form, text="Имя:").grid(row=0, column=0, sticky="w")
        ttk.Entry(form, textvariable=self.dev_name_var).grid(
            row=0, column=1, sticky="ew"
        )

        ttk.Label(form, text="IPv4:").grid(row=0, column=2, sticky="w")
        ttk.Entry(form, textvariable=self.dev_ipv4_var).grid(
            row=0, column=3, sticky="ew"
        )

        # --- строка 1 ---
        ttk.Label(form, text="Приоритет:").grid(row=1, column=0, sticky="w")

        priority_labels = [label for (label, _code, _enum) in PRIORITY_OPTIONS]
        priority_codes = [code for (_label, code, _enum) in PRIORITY_OPTIONS]
        # отображение кода -> label, нам нужен список label для комбобокса
        # а код мы потом будем искать по индексу
        ttk.Combobox(
            form,
            values=priority_labels,
            state="readonly",
            width=10,
            textvariable=tk.StringVar(),  # создаём временный Var для UI
        ).grid(row=1, column=1, sticky="w")

        # но проще всё-таки вручную синхронизировать выбор:
        self.priority_combobox = ttk.Combobox(
            form,
            values=priority_labels,
            state="readonly",
            width=10,
        )
        self.priority_combobox.grid(row=1, column=1, sticky="w")
        # по умолчанию выберем "Высокий"
        try:
            idx_default = priority_codes.index("high")
        except ValueError:
            idx_default = 0
        self.priority_combobox.current(idx_default)

        ttk.Checkbutton(
            form, text="Неисправное", variable=self.dev_is_faulty_var
        ).grid(row=1, column=2, sticky="w")
        ttk.Checkbutton(
            form, text="Резервное", variable=self.dev_is_reserve_var
        ).grid(row=1, column=3, sticky="w")

        # --- строка 2 ---
        ttk.Label(form, text="Uptime, сек:").grid(row=2, column=0, sticky="w")
        ttk.Entry(form, textvariable=self.dev_uptime_var).grid(
            row=2, column=1, sticky="ew"
        )

        ttk.Label(form, text="Ожидание, сек:").grid(row=2, column=2, sticky="w")
        ttk.Entry(form, textvariable=self.dev_standby_var).grid(
            row=2, column=3, sticky="ew"
        )

        # --- строка 3 ---
        ttk.Label(form, text="Неисправность:").grid(row=3, column=0, sticky="w")
        ttk.Entry(form, textvariable=self.dev_fault_var).grid(
            row=3, column=1, columnspan=3, sticky="ew"
        )

        # --- строка 4: кнопки ---
        ttk.Button(
            form, text="Upsert устройства", command=self.on_upsert_device
        ).grid(row=4, column=0, columnspan=2, sticky="w", pady=(4, 0))

        ttk.Button(
            form, text="Обновить список", command=self.refresh_devices
        ).grid(row=4, column=2, columnspan=2, sticky="e", pady=(4, 0))

        # --- таблица устройств ---
        columns = ("ipv4", "name", "priority", "faulty", "reserve")
        self.devices_tree = ttk.Treeview(
            frame, columns=columns, show="headings", height=10
        )
        self.devices_tree.heading("ipv4", text="IPv4")
        self.devices_tree.heading("name", text="Имя")
        self.devices_tree.heading("priority", text="Приоритет")
        self.devices_tree.heading("faulty", text="Неиспр.")
        self.devices_tree.heading("reserve", text="Резерв")

        widths = (110, 150, 90, 70, 70)
        for col, w in zip(columns, widths):
            self.devices_tree.column(col, width=w, anchor="center")

        self.devices_tree.grid(row=1, column=0, sticky="nsew")

    def _build_workers_frame(self, parent: ttk.Frame) -> None:
        """@brief Создать панель работы с работниками."""
        frame = ttk.LabelFrame(parent, text="Работники")
        frame.grid(row=1, column=1, sticky="nsew", padx=(4, 0))
        frame.columnconfigure(0, weight=1)
        frame.rowconfigure(1, weight=1)

        form = ttk.Frame(frame)
        form.grid(row=0, column=0, sticky="ew", pady=(0, 4))
        form.columnconfigure(1, weight=1)

        ttk.Label(form, text="Имя:").grid(row=0, column=0, sticky="w")
        ttk.Entry(form, textvariable=self.worker_name_var).grid(
            row=0, column=1, sticky="ew"
        )

        ttk.Label(form, text="Макс. задач:").grid(row=1, column=0, sticky="w")
        ttk.Entry(form, textvariable=self.worker_max_jobs_var, width=6).grid(
            row=1, column=1, sticky="w"
        )

        ttk.Label(form, text="Навык:").grid(row=2, column=0, sticky="w")
        ttk.Entry(form, textvariable=self.worker_skill_var).grid(
            row=2, column=1, sticky="ew"
        )

        ttk.Button(
            form, text="Добавить работника", command=self.on_add_worker
        ).grid(row=3, column=0, sticky="w", pady=(4, 0))

        ttk.Button(
            form, text="Обновить список", command=self.refresh_workers
        ).grid(row=3, column=1, sticky="e", pady=(4, 0))

        columns = ("id", "name", "max_jobs", "skill")
        self.workers_tree = ttk.Treeview(
            frame, columns=columns, show="headings", height=10
        )

        titles = ("ID", "Имя", "Макс. задач", "Навык")
        widths = (50, 150, 90, 140)
        for col, title, width in zip(columns, titles, widths):
            self.workers_tree.heading(col, text=title)
            self.workers_tree.column(col, width=width, anchor="center")

        self.workers_tree.grid(row=1, column=0, sticky="nsew")

    def _build_jobs_frame(self, parent: ttk.Frame) -> None:
        """@brief Создать панель задач (jobs)."""
        frame = ttk.LabelFrame(parent, text="Задачи")
        frame.grid(row=2, column=0, columnspan=2, sticky="nsew", pady=(8, 0))
        frame.columnconfigure(0, weight=1)
        frame.rowconfigure(1, weight=1)

        form = ttk.Frame(frame)
        form.grid(row=0, column=0, sticky="ew", pady=(0, 4))
        form.columnconfigure(3, weight=1)

        ttk.Label(form, text="Поломка: IPv4").grid(row=0, column=0, sticky="w")
        ttk.Entry(form, textvariable=self.breakdown_ipv4_var, width=16).grid(
            row=0, column=1, sticky="w"
        )

        ttk.Label(form, text="Описание:").grid(row=0, column=2, sticky="w")
        ttk.Entry(form, textvariable=self.breakdown_fault_var).grid(
            row=0, column=3, sticky="ew"
        )

        ttk.Button(form, text="Создать задачу", command=self.on_breakdown).grid(
            row=0, column=4, padx=4
        )
        ttk.Button(
            form, text="Обновить список задач", command=self.refresh_jobs
        ).grid(row=0, column=5, padx=4)

        columns = ("id", "device", "worker", "status", "fault")
        self.jobs_tree = ttk.Treeview(
            frame, columns=columns, show="headings", height=10
        )

        titles = ("ID", "Устройство", "Работник", "Статус", "Неисправность")
        widths = (50, 120, 80, 90, 260)
        for col, title, width in zip(columns, titles, widths):
            self.jobs_tree.heading(col, text=title)
            self.jobs_tree.column(col, width=width, anchor="center")

        self.jobs_tree.grid(row=1, column=0, sticky="nsew")



    def _require_engine(self) -> esm_py.Engine | None:
        """
        @brief Убедиться, что Engine инициализирован.

        @return Объект Engine или None, если нужно прервать действие.
        """
        if self.engine is None:
            messagebox.showerror("Ошибка", "Сначала откройте базу данных.")
            return None
        return self.engine



    def on_open_db(self) -> None:
        """@brief Открыть/создать БД и выполнить миграции."""
        db_path = self.db_path_var.get().strip()
        if not db_path:
            messagebox.showerror("Ошибка", "Путь к файлу БД не может быть пустым.")
            return

        try:
            self.engine = esm_py.Engine(db_path)
            self.engine.migrate()
        except Exception as exc:  # noqa: BLE001
            messagebox.showerror("Ошибка", f"Не удалось открыть БД: {exc}")
            self.engine = None
            return

        self.refresh_workers()
        self.refresh_devices()
        self.refresh_jobs()
        messagebox.showinfo("Готово", "База данных открыта и миграции применены.")

    def on_upsert_device(self) -> None:
        """@brief Добавить или обновить устройство."""
        engine = self._require_engine()
        if engine is None:
            return

        name = self.dev_name_var.get().strip()
        ipv4 = self.dev_ipv4_var.get().strip()
        if not name or not ipv4:
            messagebox.showerror("Ошибка", "Имя и IPv4 обязательны.")
            return

        try:
            address = esm_py.ipv4_to_u32(ipv4)
        except Exception as exc:  # noqa: BLE001
            messagebox.showerror("Ошибка", f"Неверный IPv4: {exc}")
            return

        try:
            uptime = int(self.dev_uptime_var.get().strip() or "0")
            standby = int(self.dev_standby_var.get().strip() or "0")
        except ValueError:
            messagebox.showerror("Ошибка", "Uptime и ожидание должны быть целыми.")
            return

        # определяем выбранный приоритет
        label_selected = self.priority_combobox.get()
        # ищем по label соответствующий enum
        priority_enum = getattr(esm_py.ServicePriority, 'None')
        for label, _code, enum_val in PRIORITY_OPTIONS:
            if label == label_selected:
                priority_enum = enum_val
                break

        is_faulty = bool(self.dev_is_faulty_var.get())
        is_reserve = bool(self.dev_is_reserve_var.get())
        fault_text = self.dev_fault_var.get().strip()

        try:
            if is_faulty:
                device = esm_py.FaultyDeviceEx(
                    name, address, priority_enum, fault_text or "Unknown fault"
                )
            elif is_reserve:
                device = esm_py.ReserveDevice(
                    name, address, priority_enum, uptime, standby
                )
            else:
                device = esm_py.HealthyDevice(
                    name, address, priority_enum, uptime
                )

            engine.upsert_device(device)
        except Exception as exc:  # noqa: BLE001
            messagebox.showerror("Ошибка", f"Не удалось сохранить устройство: {exc}")
            return

        self.refresh_devices()

    def on_add_worker(self) -> None:
        """@brief Добавить нового работника."""
        engine = self._require_engine()
        if engine is None:
            return

        name = self.worker_name_var.get().strip()
        if not name:
            messagebox.showerror("Ошибка", "Имя работника не может быть пустым.")
            return

        try:
            max_jobs = int(self.worker_max_jobs_var.get().strip() or "1")
        except ValueError:
            messagebox.showerror("Ошибка", "Макс. задач — целое число.")
            return

        skill = self.worker_skill_var.get().strip()

        try:
            engine.add_worker(name, max_jobs, skill)
        except Exception as exc:  # noqa: BLE001
            messagebox.showerror("Ошибка", f"Не удалось добавить работника: {exc}")
            return

        self.refresh_workers()

    def on_breakdown(self) -> None:
        """@brief Зафиксировать поломку и создать задачу."""
        engine = self._require_engine()
        if engine is None:
            return

        ipv4 = self.breakdown_ipv4_var.get().strip()
        fault = self.breakdown_fault_var.get().strip() or "Unknown fault"
        if not ipv4:
            messagebox.showerror("Ошибка", "IPv4 обязателен.")
            return

        try:
            address = esm_py.ipv4_to_u32(ipv4)
        except Exception as exc:  # noqa: BLE001
            messagebox.showerror("Ошибка", f"Неверный IPv4: {exc}")
            return

        try:
            job_id = engine.breakdown(address, fault)
        except Exception as exc:  # noqa: BLE001
            messagebox.showerror("Ошибка", f"Не удалось создать задачу: {exc}")
            return

        self.refresh_jobs()
        messagebox.showinfo("Создана задача", f"ID новой задачи: {job_id}")



    def refresh_workers(self) -> None:
        """@brief Перечитать и отобразить список работников."""
        engine = self.engine
        if engine is None:
            return

        for item in self.workers_tree.get_children():
            self.workers_tree.delete(item)

        try:
            workers = engine.list_workers()
        except Exception:
            return

        for w in workers:
            self.workers_tree.insert(
                "",
                "end",
                values=(w.id, w.name, w.max_jobs, w.skill),
            )

    def refresh_devices(self) -> None:
        """@brief Перечитать и отобразить список устройств."""
        engine = self.engine
        if engine is None:
            return

        for item in self.devices_tree.get_children():
            self.devices_tree.delete(item)

        try:
            devices = engine.list_devices()
        except Exception:
            return

        for d in devices:
            ipv4 = esm_py.u32_to_ipv4(d.address)
            label = PRIORITY_ENUM_TO_LABEL.get(d.priority, "Нет")
            self.devices_tree.insert(
                "",
                "end",
                values=(
                    ipv4,
                    d.name,
                    label,
                    "Да" if d.is_faulty else "Нет",
                    "Да" if d.is_reserve else "Нет",
                ),
            )

    def refresh_jobs(self) -> None:
        """@brief Перечитать и отобразить список задач."""
        engine = self.engine
        if engine is None:
            return

        for item in self.jobs_tree.get_children():
            self.jobs_tree.delete(item)

        try:
            jobs = engine.list_jobs()
        except Exception:
            return

        for j in jobs:
            worker = j.worker_id if j.worker_id is not None else ""
            self.jobs_tree.insert(
                "",
                "end",
                values=(j.id, j.device_address, worker, j.status, j.fault),
            )


def main() -> None:
    """@brief Точка входа в GUI-приложение."""
    app = App()
    app.mainloop()


if __name__ == "__main__":
    main()
