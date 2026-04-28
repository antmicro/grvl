# Metadata

Metadata gives an ability to store complex data and information for later use in GUI components. Metadata values can be accessed by calling components' member functions `AddMetadata(key, value)` and `GetMetadata(key)`:

```javascript
button.AddMetadata("description", "grvl is very cool")
...
const description = button.GetMetadata("description")
```
