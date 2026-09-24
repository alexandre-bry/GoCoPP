import os
import re
import sys

iterator_h = os.path.join(os.environ.get('PREFIX', ''), 'include', 'CGAL', 'boost', 'graph', 'iterator.h')

if not os.path.exists(iterator_h):
    print("Warning: CGAL iterator.h not found, skipping patch")
    sys.exit(0)

with open(iterator_h, 'r') as f:
    content = f.read()

if 'return (g != nullptr)' in content:
    print("CGAL already patched")
    sys.exit(0)

new_content = re.sub(
    r'return \(\! \(this->base\(\) == nullptr\)\);',
    'return (g != nullptr);',
    content
)

with open(iterator_h, 'w') as f:
    f.write(new_content)

print("CGAL patched successfully!")