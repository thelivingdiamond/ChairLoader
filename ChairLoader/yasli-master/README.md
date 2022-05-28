## yasli - easy C++ serialization

Highlights:

- Write serialization once, access multiple formats:
	* JSON - enjoy static typing while using interchangable format.
	* Binary - compact and efficient storage.
	* QPropertyTree - powerful property grid.
- Good support for composite types and STL containers.
- Support of polymorphic types and enum-annotations.
- Serialization of private fields, non-intrusive serialization.
- Can be extended without modifying core library.
- Portable.

Quick example:

```cpp
enum Position {
   ENGINEER,
   MANAGER,
};

struct Entry {
	std::string name = "Foo Bar";
	int age = 25;
	Position position = ENGINEER;

	void serialize(yasli::Archive& ar) {
		ar(name, "name");
		ar(age, "age");
		ar(position, "position");
	}
};

struct Document {
	std::vector<Entry> entries = { Entry() };

	void serialize(yasli::Archive& ar) {
		ar(entries, "entries");
	}
};

YASLI_ENUM_BEGIN(Position, "Position")
YASLI_ENUM(ENGINEER, "engineer", "Engineer")
YASLI_ENUM(MANAGER, "manager", "Manager")
YASLI_ENUM_END()
```

Document instance, serialized through JSON-archive will look like this:

```json
{ "entries": [ { "name": "Foo Bar", "age": 25, "position": "engineer" } ] }
```

See [Documentation](https://github.com/koalefant/yasli/blob/master/doc/manual.adoc) for further details.

## QPropertyTree - property grid widget for Qt

Features:

- Uses the same `serialize`-methods to fetch/update data.
- Collection operations: addition, removal and reordering with Drag & Drop.
- Enumerations appear as combo-boxes.
- Access to polymorphic types.
- Extendable.
