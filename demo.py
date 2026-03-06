import equipment_status_monitoring as esm

print("esm version:", esm.version)

# создаём движок системы
engine = esm.Engine("demo.db")

# запускаем миграции БД
engine.migrate()

# добавим работников
w1 = engine.add_worker("Alice", 3, "mechanic")
w2 = engine.add_worker("Bob", 2, "electric")

print("workers:", engine.list_workers())

# посмотрим устройства
print("devices:", engine.list_devices())

# посмотрим задания
print("jobs:", engine.list_jobs())