"""
@brief Юнит-тесты для Boost.Python биндингов esm_py.
"""

import os
import tempfile
import unittest

import esm_py


class BindingsTests(unittest.TestCase):
    """
    @brief Проверяет базовые сценарии работы :class:`esm_py.Engine` и типов.
    """

    def setUp(self) -> None:
        """
        @brief Создать временную БД и применить миграции.
        """
        fd, path = tempfile.mkstemp(prefix="esm_test_", suffix=".db")
        os.close(fd)
        self.db_path = path

        self.engine = esm_py.Engine(self.db_path)
        self.engine.migrate()

    def tearDown(self) -> None:
        """
        @brief Удалить временный файл БД.
        """
        try:
            os.remove(self.db_path)
        except OSError:
            pass

    def test_add_worker_and_list(self) -> None:
        """
        @brief Проверка добавления и выборки работников.
        """
        worker_id = self.engine.add_worker("Alice", 2, "electrician")
        self.assertIsInstance(worker_id, int)

        workers = self.engine.list_workers()
        self.assertEqual(len(workers), 1)

        w = workers[0]
        self.assertEqual(w.name, "Alice")
        self.assertEqual(w.max_jobs, 2)
        self.assertEqual(w.skill, "electrician")

    def test_upsert_device_and_breakdown(self) -> None:
        """
        @brief Проверка добавления устройства и создания задачи.
        """
        addr = esm_py.ipv4_to_u32("10.0.0.1")
        dev = esm_py.HealthyDevice(
            "Pump-1",
            addr,
            esm_py.ServicePriority.High,
            3600,
        )

        self.engine.upsert_device(dev)

        devices = self.engine.list_devices()
        self.assertEqual(len(devices), 1)
        d = devices[0]
        self.assertEqual(d.address, addr)
        self.assertEqual(d.name, "Pump-1")
        self.assertEqual(d.priority, esm_py.ServicePriority.High)

        job_id = self.engine.breakdown(addr, "Leak detected")
        self.assertIsInstance(job_id, int)

        jobs = self.engine.list_jobs()
        self.assertGreaterEqual(len(jobs), 1)

        job = jobs[0]
        self.assertEqual(job.device_address, addr)
        self.assertEqual(job.status, esm_py.JobStatus.Open)
        self.assertEqual(job.fault, "Leak detected")


if __name__ == "__main__":
    unittest.main()
