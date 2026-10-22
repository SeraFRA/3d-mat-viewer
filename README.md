# 3D Mathematical Engine Core (C)

Motore grafico e computazionale 3D per la rappresentazione e triangolazione di funzioni matematiche in tre variabili $f(x, y, z)$. Il nucleo computazionale ad altissime prestazioni è scritto in C puro, progettato per interfacciarsi con un visualizzatore 3D (ad es. in Python via `ctypes` / OpenGL / VisPy).

---

## 🏗️ Architettura del Nucleo C

Il sistema è strutturato secondo i principi di massima efficienza cache-friendly per l'elaborazione di campi scalari e mesh 3D:

1. **`DynamicList` ([`include/dynamic_list.h`](file:///C:/Users/franc/Desktop/Progetti/progetto%203d/include/dynamic_list.h), [`src/dynamic_list.c`](file:///C:/Users/franc/Desktop/Progetti/progetto%203d/src/dynamic_list.c))**:
   - Vettore dinamico generico a memoria contigua con fattore di crescita esponenziale.
   - Fornisce operazioni `init`, `push`, `pop`, `peek`, `get`, `clear`, `free`.

2. **Tokenizer & Lexer ([`include/token.h`](file:///C:/Users/franc/Desktop/Progetti/progetto%203d/include/token.h), [`src/token.c`](file:///C:/Users/franc/Desktop/Progetti/progetto%203d/src/token.c))**:
   - Scansiona la stringa matematica gestendo costanti (`pi`, `e`), numeri floating point in notazione scientifica, operatori binari e unari (`+`, `-`, `*`, `/`, `^`, negazione unaria).
   - Identifica le chiamate a funzioni matematiche (`sin`, `cos`, `tan`, `sqrt`, `exp`, `log`, `abs`, `min`, `max`, ecc.).
   - Supporta la **moltiplicazione implicita** (ad es. `2x`, `x y`, `(x+1)(y-1)`).

3. **Parser Shunting-Yard ([`include/parser.h`](file:///C:/Users/franc/Desktop/Progetti/progetto%203d/include/parser.h), [`src/parser.c`](file:///C:/Users/franc/Desktop/Progetti/progetto%203d/src/parser.c))**:
   - Converte l'espressione in notazione postfissa (RPN / Reverse Polish Notation) conservando la precedenza corretta degli operatori e l'associatività a destra.
   - Registra le variabili uniche (es. `x`, `y`, `z`) in una **lista dinamica di variabili** (`pf->variables`).
   - Produce una **lista dinamica di istruzioni** (`pf->instructions`), azzerando il costo di parsing durante i cicli di rendering 3D.

4. **Valutatore a Stack ([`include/evaluator.h`](file:///C:/Users/franc/Desktop/Progetti/progetto%203d/include/evaluator.h), [`src/evaluator.c`](file:///C:/Users/franc/Desktop/Progetti/progetto%203d/src/evaluator.c))**:
   - Esegue la sequenza di istruzioni in tempo $O(N)$ lineare senza ricorsione.
   - Fornisce `eval_function_3d(pf, x, y, z, &result)` ottimizzato per lo spazio 3D.
   - Fornisce `eval_function_grid_3d(...)` per campionare griglie voxel tridimensionali contigue.

5. **Mesh 3D & Wavefront OBJ ([`include/mesh.h`](file:///C:/Users/franc/Desktop/Progetti/progetto%203d/include/mesh.h), [`src/mesh.c`](file:///C:/Users/franc/Desktop/Progetti/progetto%203d/src/mesh.c))**:
   - Buffer contigui per vertici `(x, y, z)`, normali `(nx, ny, nz)` e indici di triangolazione `uint32_t`.
   - Funzione di esportazione Wavefront `.OBJ` nativa (`mesh3d_export_obj`).

6. **Algoritmo Marching Cubes ([`include/marching_cubes.h`](file:///C:/Users/franc/Desktop/Progetti/progetto%203d/include/marching_cubes.h), [`src/marching_cubes.c`](file:///C:/Users/franc/Desktop/Progetti/progetto%203d/src/marching_cubes.c), [`include/marching_cubes_tables.h`](file:///C:/Users/franc/Desktop/Progetti/progetto%203d/include/marching_cubes_tables.h))**:
   - Tabella canonica a 256 configurazioni per estrazione poligonale da campi scalari continui.
   - Calcolo delle normali analitiche mediante gradiente a differenze finite: $\mathbf{n} = \frac{\nabla F}{\|\nabla F\|}$.
   - Supporto nativo per **chunking** nello spazio infinito (`marching_cubes_extract_chunk`).

---

## 🚀 Compilazione ed Esecuzione Test

### 1. Esecuzione automatica con Script Batch
```cmd
build.bat
```
Lo script compila i test del parser (`build\run_tests.exe`), i test di Marching Cubes (`build\run_mc_tests.exe`), lo strumento CLI (`build\engine_parser.exe`) ed esegue entrambe le suite di test.

### 2. Esecuzione con Make (MinGW / GCC)
```bash
make
make test
```

### 3. Utilizzo da Riga di Comando
```cmd
build\engine_parser.exe "x^2 + y^2 + z^2 - 1" "sin(x)*cos(y) + sin(y)*cos(z) + sin(z)*cos(x)"
```

Output:
```text
=== 3D MARCHING CUBES ISOSURFACE EXTRACTION ===
  Extracted Mesh: 3912 vertices, 1304 triangles (bounds [-2, 2], res 24x24x24)
  [SUCCESS] Exported 3D mesh to 'build/demo_sphere.obj'
```
