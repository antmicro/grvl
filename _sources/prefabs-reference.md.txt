# Prefabs

Prefabs are user-defined composites of already existing GUI components and can be used to instantiate complex structures at runtime. By using prefabs you can create new instances of hierarchical GUI components without delving into their internals.

Prefabs can be defined in application's XML layout:

```xml
<prefab id="event" width="95" height="60" borderType="left" >

    <label id="name" text="team planning" font="mona16" x="0" y="0" width="138" height="35" horizontalOffset="8" textColor="#FF47A8FF" alignment="left" />
    <label id="description" font="mona14" x="0" y="25" width="138" height="35" horizontalOffset="8" textColor="#FF47A8FF" alignment="left" visibility="false" />

</prefab>
```

and then later instantiated at runtime with:

```javascript
const prefab = GetPrefabById("event")
const event = prefab.Clone()
events.AddElement(event)
```
