import matplotlib.pyplot as plt
import numpy as np
from collections import Counter
import re
from scipy.optimize import curve_fit

def zipf(r, C, s):
    return C * r**-s

with open('ar.txt', 'r', encoding='utf-8') as file:
    text = file.read().lower()

words = re.findall(r'\b[\w-]+\b', text)
words = [word for word in words if word.isalpha()]

word_counts = Counter(words)
most_common = word_counts.most_common()

ranks = np.arange(1, len(most_common) + 1)
frequencies = np.array([count for word, count in most_common])

popt, _ = curve_fit(zipf, ranks, frequencies, p0=[max(frequencies), 1.0])
C_opt, s_opt = popt

plt.figure(figsize=(10, 6))
plt.loglog(ranks, frequencies, 'o', markersize=3, label='Slowa')
plt.loglog(ranks, zipf(ranks, C_opt, s_opt), 'r-', label=f'Zipf: s={s_opt:.2f}')
plt.xlabel('Ranga')
plt.ylabel('Czestotliwosc')
plt.title('Analiza Zipfa')
plt.legend()
plt.grid(True)
plt.show()

print(f"Exponenta Zipfa: {s_opt:.3f}")
print(f"Top 10 slowa: {most_common[:10]}")