{#conf}

# conf

## Variables

| Return | Name | Description |
|--------|------|-------------|
| `str` | [`project`](#project)  |  |
| `str` | [`copyright`](#copyright)  |  |
| `str` | [`author`](#author)  |  |
| `list` | [`extensions`](#extensions)  |  |
| `list` | [`templates_path`](#templates_path)  |  |
| `list` | [`exclude_patterns`](#exclude_patterns)  |  |
| `str` | [`html_theme`](#html_theme)  |  |
| `list` | [`html_static_path`](#html_static_path)  |  |
| `str` | [`breathe_default_project`](#breathe_default_project)  |  |
| `dict` | [`breathe_projects`](#breathe_projects)  |  |

---

{#project}

### project

```cpp
str project =  'QtRealTimePlot'
```

---

{#copyright}

### copyright

```cpp
str copyright =  '2026, Laboratorio Gluon'
```

---

{#author}

### author

```cpp
str author =  'Laboratorio Gluon'
```

---

{#extensions}

### extensions

```cpp
list extensions =  ['sphinx.ext.autodoc','breathe']
```

---

{#templates_path}

### templates_path

```cpp
list templates_path =  ['_templates']
```

---

{#exclude_patterns}

### exclude_patterns

```cpp
list exclude_patterns =  []
```

---

{#html_theme}

### html_theme

```cpp
str html_theme =  'alabaster'
```

---

{#html_static_path}

### html_static_path

```cpp
list html_static_path =  ['_static']
```

---

{#breathe_default_project}

### breathe_default_project

```cpp
str breathe_default_project =  "my_project"
```

---

{#breathe_projects}

### breathe_projects

```cpp
dict breathe_projects =  {"myproject": "."}
```

