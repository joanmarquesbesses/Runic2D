import os

# 1. Carpetes que NO volem que la IA llegeixi (per estalviar espai i confusió)
IGNORE_DIRS = {'.git', '.vs', 'vendor', 'bin', 'bin-int', 'Dependencies', 'out'}

# 2. Tipus de fitxers que ens interessen (C++ i Shaders)
VALID_EXTENSIONS = {'.h', '.cpp', '.glsl', '.hlsl', '.lua'}

output_file = 'context_projecte.txt'

def generar_context():
    with open(output_file, 'w', encoding='utf-8') as outfile:
        # Recorrem totes les carpetes i subcarpetes
        for root, dirs, files in os.walk('.'):
            # Filtrem les carpetes que volem ignorar
            dirs[:] = [d for d in dirs if d not in IGNORE_DIRS]
            
            for file in files:
                if any(file.endswith(ext) for ext in VALID_EXTENSIONS):
                    filepath = os.path.join(root, file)
                    try:
                        with open(filepath, 'r', encoding='utf-8') as infile:
                            content = infile.read()
                            # Escrivim una capçalera clara per a la IA
                            outfile.write(f"\n{'='*60}\n")
                            outfile.write(f"FITXER: {filepath}\n")
                            outfile.write(f"{'='*60}\n\n")
                            outfile.write(content)
                            outfile.write("\n")
                    except Exception as e:
                        print(f"Error llegint {filepath}: {e}")

    print(f"✅ Context generat correctament a: {output_file}")

if __name__ == '__main__':
    generar_context()