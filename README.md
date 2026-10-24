# 3D Mathematical Engine & Viewer

Un motore grafico e computazionale ad altissime prestazioni per la rappresentazione, triangolazione e visualizzazione interattiva in tempo reale di funzioni matematiche in tre variabili $f(x, y, z) = 0$ in uno spazio tridimensionale "fintamente infinito".

L'architettura è progettata secondo i principi di massima leggerezza sull'hardware, assenza di dipendenze pesanti e consumo minimo di risorse (RAM < 20 MB, avvio istantaneo, 100+ FPS).

Il progetto è stato realizzato completamente da pochi prompt lasciati in pasto a Gemini 3.8 Flash in un loop agentico su Antigravity CLI.

---

## 🏗️ Architettura del Progetto

```
progetto 3d/
├── include/
│   ├── dynamic_list.h           # Vettore dinamico contiguo generico (cache-friendly)
│   ├── token.h                  # Tokenizer per numeri, variabili, operatori, costanti
│   ├── parser.h                 # Parser Shunting-Yard (RPN) con moltiplicazione implicita
│   ├── evaluator.h              # Valutatore matematico a stack O(N) e campionamento 3D
│   ├── mesh.h                   # Struttura mesh 3D (vertici, normali, indici) ed export OBJ
│   ├── marching_cubes_tables.h  # Tabelle canoniche a 256 configurazioni
│   ├── marching_cubes.h         # Algoritmo Marching Cubes con normali da gradiente analitico
│   └── viewer.h                 # Visualizzatore 3D OpenGL nativo con camera orbitale
├── src/
│   ├── dynamic_list.c
│   ├── token.c
│   ├── parser.c
│   ├── evaluator.c
│   ├── mesh.c
│   ├── marching_cubes.c
│   ├── viewer.c
│   └── main.c
├── tests/
│   ├── test_parser.c            # Test suite per parser e dynamic list (65 asserzioni)
│   └── test_marching_cubes.c    # Test suite per Marching Cubes (6147 asserzioni)
├── gui_launcher.py              # Launcher/Controller grafico leggero in Python (Tkinter)
├── build.bat                    # Script di compilazione e test automatico per Windows (GCC)
├── Makefile                     # Makefile per MinGW / Linux
└── README.md
```

---

## ⚡ Caratteristiche Principali

1. **Parser Matematico in C Puro**:
   - Riconosce variabili libere ($x, y, z$), costanti matematiche ($pi, e$), funzioni trigonometriche e iperboliche ($sin, cos, tan, exp, log, sqrt, abs, min, max$).
   - Moltiplicazione implicita intelligente (ad es. `2x`, `x y`, `(x+1)(y-1)`).
   - Genera una lista dinamica di istruzioni in notazione postfissa (RPN) che azzera il costo di interpretazione nei cicli di calcolo intensivo.

2. **Marching Cubes con Normali da Gradiente $\nabla F$**:
   - Triangola qualsiasi superficie implicita continua $F(x, y, z) = 0$.
   - Calcola le normali di illuminazione direttamente dal gradiente matematico continuo:
     $$\mathbf{n} = \frac{\nabla F}{\|\nabla F\|}$$
   - Supporto nativo per **chunking volumetrico** nello spazio infinito (`marching_cubes_extract_chunk`).

3. **Visualizzatore 3D Nativo Hardware-Accelerated (OpenGL)**:
   - Leggerissimo sull'hardware: **zero dipendenze esterne** (utilizza le librerie native del sistema operativo).
   - Finestra interattiva fluida con illuminazione dinamica, griglia dello spazio infinito e assi cartesiani $X$ (rosso), $Y$ (verde), $Z$ (blu).
   - Camera 3D interattiva completa:
     - **Mouse Sinistro + Drag**: rotazione orbitale (pitch & yaw).
     - **Mouse Destro + Drag**: spostamento / traslazione (pan).
     - **Rotellina Mouse**: zoom in/out.
     - **[Spazio]**: alterna modalità piena / reticolo (wireframe).
     - **[G]**: attiva/disattiva griglia infinita.
     - **[R]**: ripristina la visuale.
     - **Tasti [1] - [6]**: cambia equazione matematica istantaneamente in tempo reale.

4. **Launcher GUI in Python (`gui_launcher.py`)**:
   - Interfaccia grafica intuitiva in Python standard (Tkinter, integrata con Python) per digitare qualsiasi formula matematica e avviare il motore 3D.

---

## 🛠️ Compilazione ed Esecuzione

### 1. Compilazione Automatica e Test
Esegui da riga di comando:
```cmd
build.bat
```
Lo script compila i test unitari, l'eseguibile 3D ed esegue automaticamente le verifiche:
- **Parser & Dynamic List**: 65 / 65 test passati.
- **Marching Cubes & Mesh**: 6147 / 6147 asserzioni passate.

### 2. Avvio del Visualizzatore 3D

**Opzione A: Da riga di comando con funzione personalizzata**
```cmd
build\engine_3d.exe "sin(x)*cos(y) + sin(y)*cos(z) + sin(z)*cos(x)"
```

**Opzione B: Tramite il Launcher Python**
```cmd
py gui_launcher.py
```

**Opzione C: Modalità CLI Batch (Esportazione mesh OBJ)**
```cmd
build\engine_3d.exe --cli "x^2 + y^2 + z^2 - 1"
```
Genera direttamente i file `.obj` nella cartella `build/` pronti per l'importazione in qualsiasi software 3D.
