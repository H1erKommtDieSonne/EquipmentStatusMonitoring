import equipment_status_monitoring as esm


db = "esm_test.db"
e = esm.Engine(db)


e.migrate()


e.add_worker("A", 2, "electric")
e.add_worker("B",  1, "mechanic")

print("Workers:", e.list_workers())


d1 = esm.HealthyDevice("Router", 101, esm.ServicePriority.High, 3600)
d2 = esm.ReserveDevice("Switch", 202, esm.ServicePriority.Low,  100, 30)

e.upsert_device(d1)
e.upsert_device(d2)
print("Devices:", e.list_devices())


job_id = e.breakdown(101, "power_supply")
print("Created job id:", job_id)


e.start_repair(job_id, 1)
e.finish_repair(job_id, 1)



print("All jobs:", e.list_jobs())
print("Done jobs:", e.list_jobs(esm.JobStatus.Done))
