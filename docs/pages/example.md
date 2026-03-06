# Example

```python
import equipment_status_monitoring as esm

engine = esm.Engine("demo.db")

engine.migrate()

engine.add_worker("Alice", 3, "mechanic")

print(engine.list_workers())