import subprocess

with open('./tmp/dif_equations', 'r') as f:
    equations = [eq.replace('^', '**').rstrip() for eq in f.readlines()]

with open('./tmp/plot.gp', 'w') as f:
    f.write('set terminal pngcairo\n')
    f.write('set output "plot.png"\n')
    f.write('set terminal png size 1024,768\n')
    f.write('set xrange [-5:5]\n')
    f.write('set yrange [-3:3]\n')
    for i, equation in enumerate(equations):
        if i == 0:
            f.write(f'plot {equation} title "{equation}"')
        else:
            f.write(f', \\\n{equation} title "{equation}"')
    f.write('\n')

subprocess.run(['gnuplot', './tmp/plot.gp'])
