import pandas as pd
import matplotlib.pyplot as plt
import numpy as np

# --- Qualitätstests ---
df_quality = pd.read_csv("quality.csv")

# Plot 1: CAT-Test (durchschnittliche Lücke)
plt.figure(figsize=(8,6))
bars = plt.bar(df_quality['RNG'], df_quality['CAT_mean_gap'], color=['steelblue', 'darkorange'])
plt.axhline(17576, color='red', linestyle='--', label='Erwartete Lücke (17576)')
plt.xlabel("RNG")
plt.ylabel("Durchschnittliche Lücke")
plt.title("CAT-Test: Durchschnittliche Lücke zwischen CAT-Vorkommen (sequenziell)")
plt.legend()
plt.grid(axis='y')
plt.savefig("cat_test_quality.png")
plt.show()

# Plot 2: \chi^2-Test für Uniformität
plt.figure(figsize=(8,6))
bars = plt.bar(df_quality['RNG'], df_quality['ChiSqUniform'], color=['steelblue', 'darkorange'])
plt.xlabel("RNG")
plt.ylabel("\chi^2-Wert")
plt.title("\chi^2-Test für Uniformität (sequenziell)")
plt.grid(axis='y')
plt.savefig("chi_square_quality.png")
plt.show()

# --- Leistungstests ---
df_perf = pd.read_csv("performance.csv")
# Erzeuge eine kombinierte Beschriftung, z.B. "rand (static)"
df_perf['Label'] = df_perf['RNG'] + " (" + df_perf['Schedule'] + ")"

plt.figure(figsize=(10,6))
bars = plt.bar(df_perf['Label'], df_perf['Time'], color='cornflowerblue')
plt.xlabel("Kombination (RNG, Schedule)")
plt.ylabel("Zeit (s)")
plt.title("Leistungsvergleich: Zeit zur Erzeugung von N Zufallszahlen")
plt.xticks(rotation=45, ha="right")
plt.grid(axis='y')
plt.tight_layout()
plt.savefig("performance_comparison.png")
plt.show()
