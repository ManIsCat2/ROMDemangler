import sys
import os
from PyQt6.QtWidgets import (
    QApplication, QWidget, QVBoxLayout, QHBoxLayout, QCheckBox,
    QPushButton, QLabel, QGroupBox, QGridLayout, QFileDialog, 
    QTabWidget, QLineEdit, QTextEdit, QScrollArea
)
from PyQt6.QtCore import Qt, QProcess
from PyQt6.QtGui import QFontDatabase, QFont, QIcon, QColor, QPalette

num_to_name = {
    4: 'bbh', 5: 'ccm', 6: 'castle_inside', 7: 'hmc', 8: 'ssl', 9: 'bob',
    10: 'sl', 11: 'wdw', 12: 'jrb', 13: 'thi', 14: 'ttc', 15: 'rr',
    16: 'castle_grounds', 17: 'bitdw', 18: 'vcutm', 19: 'bitfs', 20: 'sa',
    21: 'bits', 22: 'lll', 23: 'ddd', 24: 'wf', 26: 'castle_courtyard',
    27: 'pss', 28: 'cotmc', 29: 'totwc', 30: 'bowser_1', 31: 'wmotr',
    33: 'bowser_2', 34: 'bowser_3', 36: 'ttm'
}

class DemanglerGUI(QWidget):
    def __init__(self):
        super().__init__()

        self.setWindowTitle("ROM Demangler")
        self.setMinimumSize(550, 450)
        self.apply_dark_theme()

        main_layout = QVBoxLayout(self)

        self.tabs = QTabWidget()
        main_layout.addWidget(self.tabs)

        self.tab_config = QWidget()
        self.setup_config_tab()
        self.tabs.addTab(self.tab_config, "Configuration")

        self.tab_levels = QWidget()
        self.setup_levels_tab()
        self.tabs.addTab(self.tab_levels, "Level Selection")

        self.tab_log = QWidget()
        self.setup_log_tab()
        self.tabs.addTab(self.tab_log, "Console Log")

        bottom_layout = QHBoxLayout()
        self.status_label = QLabel("Ready.")
        self.status_label.setStyleSheet("color: #aaaaaa;")
        
        self.run_btn = QPushButton("Demangle")
        self.run_btn.setFixedHeight(40)
        self.run_btn.setFixedWidth(150)
        self.run_btn.setProperty("class", "actionButton")
        self.run_btn.clicked.connect(self.toggle_process)

        bottom_layout.addWidget(self.status_label)
        bottom_layout.addStretch()
        bottom_layout.addWidget(self.run_btn)
        
        main_layout.addLayout(bottom_layout)

        self.process = None

    def apply_dark_theme(self):
        self.setStyleSheet("""
            QWidget {
                background-color: #2b2b2b;
                color: #e0e0e0;
                font-size: 10pt;
            }
            QGroupBox {
                border: 1px solid #444;
                border-radius: 5px;
                margin-top: 10px;
                padding-top: 15px;
            }
            QGroupBox::title {
                subcontrol-origin: margin;
                subcontrol-position: top left;
                padding: 0 5px;
                color: #8ab4f8;
            }
            QLineEdit {
                background-color: #1e1e1e;
                border: 1px solid #555;
                border-radius: 3px;
                padding: 4px;
            }
            QPushButton {
                background-color: #3c3f41;
                border: 1px solid #555;
                border-radius: 4px;
                padding: 5px 15px;
            }
            QPushButton:hover {
                background-color: #4b4d4f;
            }
            QPushButton[class="actionButton"] {
                background-color: #4CAF50;
                color: white;
                font-weight: bold;
                border: none;
            }
            QPushButton[class="actionButton"]:hover {
                background-color: #45a049;
            }
            QPushButton[class="cancelButton"] {
                background-color: #d32f2f;
                color: white;
                font-weight: bold;
                border: none;
            }
            QPushButton[class="cancelButton"]:hover {
                background-color: #b71c1c;
            }
            QTabWidget::pane { border: 1px solid #444; top: -1px; }
            QTabBar::tab {
                background: #3c3f41;
                border: 1px solid #444;
                padding: 8px 12px;
                margin-right: 2px;
            }
            QTabBar::tab:selected {
                background: #2b2b2b;
                border-bottom-color: #2b2b2b;
                color: #8ab4f8;
            }
            QTextEdit {
                background-color: #1e1e1e;
                border: 1px solid #444;
                font-family: Consolas, monospace;
            }
        """)

    def setup_config_tab(self):
        layout = QVBoxLayout(self.tab_config)

        file_group = QGroupBox("File Paths")
        file_layout = QGridLayout(file_group)

        self.rom_input = QLineEdit()
        self.rom_input.setPlaceholderText("Path to ROM (.z64)...")
        rom_btn = QPushButton("Browse...")
        rom_btn.clicked.connect(lambda: self.browse_file(self.rom_input, "ROM Files (*.z64)"))

        self.ram_input = QLineEdit()
        self.ram_input.setPlaceholderText("Path to RAM dump (.bin) (Optional)...")
        ram_btn = QPushButton("Browse...")
        ram_btn.clicked.connect(lambda: self.browse_file(self.ram_input, "RAM Dumps (*.bin);;All Files (*)"))

        file_layout.addWidget(QLabel("ROM:"), 0, 0)
        file_layout.addWidget(self.rom_input, 0, 1)
        file_layout.addWidget(rom_btn, 0, 2)
        
        file_layout.addWidget(QLabel("RAM:"), 1, 0)
        file_layout.addWidget(self.ram_input, 1, 1)
        file_layout.addWidget(ram_btn, 1, 2)

        layout.addWidget(file_group)

        export_group = QGroupBox("Export Options")
        export_layout = QGridLayout(export_group)

        self.checks = {}
        opts = ["actors", "sounds", "tweaks", "fix collision", "ignore segment 0", "textures", "text", "skyboxes", "behaviors"]
        for i, opt in enumerate(opts):
            cb = QCheckBox(opt.title())
            self.checks[opt] = cb
            export_layout.addWidget(cb, i // 2, i % 2)

        layout.addWidget(export_group)
        layout.addStretch()

    def setup_levels_tab(self):
        layout = QVBoxLayout(self.tab_levels)

        btn_layout = QHBoxLayout()
        sel_all = QPushButton("Select All")
        unsel_all = QPushButton("Unselect All")
        btn_layout.addWidget(sel_all)
        btn_layout.addWidget(unsel_all)
        btn_layout.addStretch()
        layout.addLayout(btn_layout)

        scroll = QScrollArea()
        scroll.setWidgetResizable(True)
        scroll.setFrameShape(QScrollArea.Shape.NoFrame)
        
        scroll_widget = QWidget()
        grid = QGridLayout(scroll_widget)
        
        self.level_checkboxes = []
        for idx, (num, name) in enumerate(num_to_name.items()):
            cb = QCheckBox(name)
            self.level_checkboxes.append((cb, num))
            grid.addWidget(cb, idx // 3, idx % 3)

        scroll.setWidget(scroll_widget)
        layout.addWidget(scroll)

        sel_all.clicked.connect(lambda: [cb.setChecked(True) for cb, _ in self.level_checkboxes])
        unsel_all.clicked.connect(lambda: [cb.setChecked(False) for cb, _ in self.level_checkboxes])

    def setup_log_tab(self):
        layout = QVBoxLayout(self.tab_log)
        self.log_output = QTextEdit()
        self.log_output.setReadOnly(True)
        layout.addWidget(self.log_output)

    def browse_file(self, line_edit, filter_str):
        path, _ = QFileDialog.getOpenFileName(self, "Select File", "", filter_str)
        if path:
            line_edit.setText(path)

    def log(self, text, color="#e0e0e0"):
        html_text = text.replace('\n', '<br>')
        self.log_output.append(f'<span style="color:{color};">{html_text}</span>')

    def toggle_process(self):
        if self.process and self.process.state() != QProcess.ProcessState.NotRunning:
            self.process.kill()
            self.status_label.setText("Process cancelled.")
            self.reset_run_button()
            self.log("--- Process Cancelled ---", "#d32f2f")
            return

        rom = self.rom_input.text().strip()
        ram = self.ram_input.text().strip()

        if not rom:
            self.status_label.setText("Error: Please select a ROM.")
            self.tabs.setCurrentWidget(self.tab_config)
            return

        args = ["--rom", rom]
        if ram:
            args.extend(["--ram", ram])

        levels = [str(lid) for cb, lid in self.level_checkboxes if cb.isChecked()]
        if levels:
            args.extend(["--levels", ",".join(levels)])

        if self.checks.get("actors") and self.checks["actors"].isChecked():
            args.extend(["--actors", "all"])
        if self.checks.get("sounds") and self.checks["sounds"].isChecked():
            args.append("--sound")
        if self.checks.get("tweaks") and self.checks["tweaks"].isChecked():
            args.append("--tweaks")
        if self.checks.get("fix collision") and self.checks["fix collision"].isChecked():
            args.append("--fix-collision")
        if self.checks.get("ignore segment 0") and self.checks["ignore segment 0"].isChecked():
            args.append("--ignore-seg-0")
        if self.checks.get("skyboxes") and self.checks["skyboxes"].isChecked():
            args.append("--skyboxes")
        if self.checks.get("textures") and self.checks["textures"].isChecked():
            args.append("--textures")
        if self.checks.get("text") and self.checks["text"].isChecked():
            args.append("--text")
        if self.checks.get("behaviors") and self.checks["behaviors"].isChecked():
            args.append("--behaviors")

        program = self.find_executable()
        if not program:
            self.status_label.setText("Error: ROMDemangler executable not found.")
            return

        self.log_output.clear()
        self.tabs.setCurrentWidget(self.tab_log)
        self.status_label.setText("Running...")
        
        self.run_btn.setText("Cancel")
        self.run_btn.setProperty("class", "cancelButton")
        self.run_btn.style().unpolish(self.run_btn)
        self.run_btn.style().polish(self.run_btn)

        self.log(f"Executing: {program} {' '.join(args)}", "#8ab4f8")

        self.process = QProcess(self)
        self.process.setProgram(program)
        self.process.setArguments(args)

        self.process.readyReadStandardOutput.connect(self.handle_stdout)
        self.process.readyReadStandardError.connect(self.handle_stderr)
        self.process.finished.connect(self.process_finished)

        self.process.start()

    def find_executable(self):
        targets = [
            "ROMDemangler.exe", "ROMDemangler",
            os.path.join("build", "ROMDemangler.exe"),
            os.path.join("build", "ROMDemangler")
        ]
        for t in targets:
            if os.path.exists(t):
                return t
        return None

    def handle_stdout(self):
        data = self.process.readAllStandardOutput()
        text = bytes(data).decode("utf-8", errors="replace").strip()
        if text:
            self.log(text)

    def handle_stderr(self):
        data = self.process.readAllStandardError()
        text = bytes(data).decode("utf-8", errors="replace").strip()
        if text:
            self.log(text, "#ff6b6b")

    def process_finished(self):
        if self.process.exitStatus() == QProcess.ExitStatus.NormalExit and self.process.exitCode() == 0:
            self.status_label.setText("Extraction completed successfully.")
            self.log("--- Process Finished Successfully ---", "#4CAF50")
        else:
            self.status_label.setText("Process exited with errors.")
        self.reset_run_button()

    def reset_run_button(self):
        self.run_btn.setText("Demangle")
        self.run_btn.setProperty("class", "actionButton")
        self.run_btn.style().unpolish(self.run_btn)
        self.run_btn.style().polish(self.run_btn)


if __name__ == '__main__':
    app = QApplication(sys.argv)
    
    font_id = QFontDatabase.addApplicationFont("gui/sm64.ttf")
    app.setWindowIcon(QIcon("gui/icon.png"))
    if font_id != -1:
        families = QFontDatabase.applicationFontFamilies(font_id)
        if families:
            app.setFont(QFont(families[0], 10))
            
    window = DemanglerGUI()
    window.show()
    sys.exit(app.exec())