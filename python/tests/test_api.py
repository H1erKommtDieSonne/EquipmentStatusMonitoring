import equipment_status_monitoring as esm


db = "esm_test.db"
e = esm.Engine(db)

# 1) миграции/создание таблиц
e.migrate()

# 2) работники
e.add_worker("Alex", 2, 5)
e.add_worker("Bob",  1, 3)
print("Workers:", e.list_workers())

# 3) устройства (upsert)
d1 = esm.HealthyDevice("Router-1", 101, esm.ServicePriority.High, 3600)
d2 = esm.ReserveDevice("Switch-1", 202, esm.ServicePriority.Low,  100, 30)

e.upsert_device(d1)
e.upsert_device(d2)
print("Devices:", e.list_devices())

# 4) поломка -> создаём job
job_id = e.breakdown(101, "power_supply")
print("Created job id:", job_id)

# 5) старт/финиш ремонта
e.start_repair(job_id, 1)     # worker_id=1 (если у тебя ids с 1)
e.finish_repair(job_id)

# 6) список джоб
print("All jobs:", e.list_jobs())
print("Done jobs:", e.list_jobs(esm.JobStatus.Done))
