# 3D Mathematical Engine Core (C)

Motore grafico e computazionale 3D per la rappresentazione di funzioni matematiche in tre variabili $f(x, y, z)$. Il nucleo computazionale ad altissime prestazioni è scritto in C puro, progettato per interfacciarsi con un visualizzatore 3D (ad es. in Python via `ctypes` / OpenGL / VisPy).

---

## 🏗️ Architettura del Nucleo C

Il sistema è strutturato secondo i principi di massima efficienza cache-friendly per l'elaborazione di campi scalari 3D:

1. **`DynamicList` (`dynamic_list.h`, `dynamic_list.c`)**:
   - Vettore dinamico generico a memoria contigua con fattore di crescita esponenziale.
   - Fornisce operazioni `init`, `push`, `pop`, `peek`, `get`, `clear`, `free`.
   - Utilizzato sia per memorizzare i token, sia per la lista dinamica delle variabili rilevate, sia per il bytecode RPN delle operazioni.

2. **Tokenizer & Lexer (`token.h`, `token.c`)**:
   - Scansiona la stringa matematica gestendo costanti (`pi`, `e`), numeri floating point in notazione scientifica, operatori binari e unari (`+`, `-`, `*`, `/`, `^`, unary `-`).
   - Identifica le chiamate a funzioni matematiche (`sin`, `cos`, `tan`, `sqrt`, `exp`, `log`, `abs`, `min`, `max`, ecc.).
   - Supporta la **moltiplicazione implicita** (ad es. `2x`, `x y`, `(x+1)(y-1)`).

3. **Parser Shunting-Yard (`parser.h`, `parser.c`)**:
   - Converte l'espressione in notazione postfissa (RPN / Reverse Polish Notation) conservando la precedenza corretta degli operatori e l'associatività a destra di potenze e negazioni.
   - Isola e registra automaticamente le variabili uniche (es. `x`, `y`, `z`) in una **lista dinamica di variabili** (`pf->variables`), assegnando a ciascuna uno slot indice numerico per lookup a tempo $O(1)$.
   - Produce una **lista dinamica di istruzioni** (`pf->instructions`), azzerando il costo di parsing durante i cicli di rendering 3D.

4. **Valutatore a Stack (`evaluator.h`, `evaluator.c`)**:
   - Esegue la sequenza di istruzioni in tempo $O(N)$ lineare senza ricorsione.
   - Fornisce `eval_function_3d(pf, x, y, z, &result)` ottimizzato per lo spazio 3D.
   - Fornisce `eval_function_grid_3d(...)` per calcolare griglie voxel tridimensionali contigue (fondamentale per Marching Cubes / Raymarching / isosuperfici).

---

## 🚀 Compilazione ed Esecuzione Test

### 1. Esecuzione automatica con Script Batch
```cmd
build.bat
```
Lo script compila i test unitari (`build\run_tests.exe`), lo strumento CLI (`build\engine_parser.exe`) ed esegue immediatamente la suite di test completa.

### 2. Esecuzione con Make (MinGW / GCC)
```bash
make
make test
```

### 3. Utilizzo Interattivo da Riga di Comando
Puoi passare qualsiasi funzione a tre variabili come argomento:
```cmd
build\engine_parser.exe "x^2 + y^2 + z^2 - 1" "sin(x)*cos(y) + z" "exp(-(x^2+y^2+z^2))"
```

Output di esempio:
```text
Parsing expression: "x^2 + y^2 + z^2 - 1"
=== TOKENS (13) ===
[00] [Token VARIABLE name='x' pos=0]
[01] [Token OP_POW (^) pos=1]
[02] [Token NUMBER val=2 pos=2]
...
=== VARIABLES (3) ===
Slot #0: 'x'
Slot #1: 'y'
Slot #2: 'z'

=== POSTFIX INSTRUCTIONS / OPERATIONS (13) ===
[00] LOAD_VAR 'x' (slot 0)
[01] LOAD_NUM 2
[02] OP_POW (^)
...
=== 3D SPACE EVALUATIONS ===
  f(0, 0, 0) = -1
  f(1, 0, 0) = 0
  f(0, 1, 0) = 0
  f(0, 0, 1) = 0
  f(1, 1, 1) = 2
```

---

## 🧪 Test Suite Automatica
La suite copre **65 asserzioni rigorose**:
- Operazioni e ridimensionamento `DynamicList`.
- Precedenza operatori (`2 + 3 * 4 == 14`) e associatività a destra (`2 ^ 3 ^ 2 == 512`).
- Gestione corretta di meno unario (`-5 + 8`, `-(-x)`).
- Riconoscimento variabili `x, y, z` e mappatura slot.
- Funzioni matematiche trigonometriche, radici, esponenziali (`sin(x)^2 + cos(x)^2 == 1`).
- Moltiplicazione implicita (`2x + 3y - 4z`).
- Campionamento griglia volumetrica 3D ($5 \times 5 \times 5$).
- Gestione errori di sintassi, parentesi sbilanciate, divisione per zero.
