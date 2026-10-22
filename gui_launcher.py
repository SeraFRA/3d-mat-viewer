#!/usr/bin/env python3
"""
3D Math Engine - Python Launcher & Controller
A lightweight Python GUI to interact with the C 3D Graphics Engine.
"""

import sys
import os
import subprocess
import tkinter as tk
from tkinter import ttk, messagebox

PRESETS = [
    ("Sfera", "x^2 + y^2 + z^2 - 1"),
    ("Gyroid (Spazio Infinito)", "sin(x) * cos(y) + sin(y) * cos(z) + sin(z) * cos(x)"),
    ("Sella (Paraboloide Iperbolico)", "x^2 - y^2 - z"),
    ("Toro (Ciambella)", "(x^2 + y^2 + z^2 + 0.8^2 - 0.3^2)^2 - 4 * 0.8^2 * (x^2 + y^2)"),
    ("Onda Circolare", "sqrt(x^2 + y^2) - cos(z)"),
    ("Cono Doppio", "x^2 + y^2 - z^2")
]

class MathEngineApp:
    def __init__(self, root):
        self.root = root
        self.root.title("3D Mathematical Engine - Control Panel")
        self.root.geometry("640x480")
        self.root.minsize(500, 380)

        # Base path to C executable
        script_dir = os.path.dirname(os.path.abspath(__file__))
        self.exe_path = os.path.join(script_dir, "build", "engine_3d.exe")

        self.setup_ui()

    def setup_ui(self):
        style = ttk.Style()
        style.theme_use("clam")

        # Main frame
        main_frame = ttk.Frame(self.root, padding="16")
        main_frame.pack(fill=tk.BOTH, expand=True)

        # Title
        title_label = ttk.Label(
            main_frame,
            text="Motore Grafico 3D per Funzioni Matematiche",
            font=("Helvetica", 14, "bold")
        )
        title_label.pack(pady=(0, 8))

        subtitle = ttk.Label(
            main_frame,
            text="Nucleo C ad alte prestazioni con Marching Cubes & OpenGL nativo",
            font=("Helvetica", 9, "italic")
        )
        subtitle.pack(pady=(0, 16))

        # Formula input frame
        input_frame = ttk.LabelFrame(main_frame, text=" Funzione Implicita f(x, y, z) = 0 ", padding="12")
        input_frame.pack(fill=tk.X, pady=(0, 12))

        lbl = ttk.Label(input_frame, text="Inserisci la formula:")
        lbl.pack(anchor=tk.W, pady=(0, 4))

        self.formula_var = tk.StringVar(value=PRESETS[1][1])
        self.entry = ttk.Entry(input_frame, textvariable=self.formula_var, font=("Consolas", 11))
        self.entry.pack(fill=tk.X, pady=(0, 8))

        # Preset buttons frame
        preset_frame = ttk.LabelFrame(main_frame, text=" Preset Funzioni 3D ", padding="8")
        preset_frame.pack(fill=tk.X, pady=(0, 16))

        grid_frame = ttk.Frame(preset_frame)
        grid_frame.pack(fill=tk.X)

        for i, (name, formula) in enumerate(PRESETS):
            btn = ttk.Button(
                grid_frame,
                text=name,
                command=lambda f=formula: self.select_preset(f)
            )
            row = i // 3
            col = i % 3
            btn.grid(row=row, column=col, padx=4, pady=4, sticky="ew")
            grid_frame.columnconfigure(col, weight=1)

        # Launch button
        launch_btn = tk.Button(
            main_frame,
            text="🚀  Avvia Visualizzatore 3D (C / OpenGL)",
            font=("Helvetica", 12, "bold"),
            bg="#007acc",
            fg="white",
            activebackground="#005999",
            activeforeground="white",
            relief=tk.FLAT,
            padx=12,
            pady=10,
            command=self.launch_viewer
        )
        launch_btn.pack(fill=tk.X, pady=(0, 12))

        # Info Box
        info_frame = ttk.LabelFrame(main_frame, text=" Comandi nel Visualizzatore 3D ", padding="8")
        info_frame.pack(fill=tk.BOTH, expand=True)

        info_text = (
            "• Mouse Sinistro + Drag: Ruota la visuale attorno al centro (Orbit)\n"
            "• Mouse Destro + Drag: Sposta la visuale (Pan)\n"
            "• Rotellina del Mouse: Zoom In / Out nello spazio 3D\n"
            "• [Spazio]: Alterna modalità Piena (Solid) / Reticolo (Wireframe)\n"
            "• [G]: Mostra/Nascondi la griglia dello spazio infinito\n"
            "• [R]: Ripristina la posizione della telecamera\n"
            "• Tasti [1] - [6]: Cambia funzione matematica all'istante"
        )
        info_lbl = ttk.Label(info_frame, text=info_text, font=("Consolas", 9), justify=tk.LEFT)
        info_lbl.pack(anchor=tk.W)

    def select_preset(self, formula):
        self.formula_var.set(formula)

    def launch_viewer(self):
        formula = self.formula_var.get().strip()
        if not formula:
            messagebox.showwarning("Attenzione", "Inserisci una formula valida!")
            return

        if not os.path.exists(self.exe_path):
            messagebox.showerror(
                "Errore",
                f"Eseguibile non trovato in:\n{self.exe_path}\nEsegui prima build.bat per compilare!"
            )
            return

        try:
            # Launch C viewer asynchronously
            subprocess.Popen([self.exe_path, formula])
        except Exception as e:
            messagebox.showerror("Errore", f"Impossibile avviare il visualizzatore:\n{e}")

if __name__ == "__main__":
    root = tk.Tk()
    app = MathEngineApp(root)
    root.mainloop()
