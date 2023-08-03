import urllib.request
from plyfile import PlyData, PlyElement
import numpy as np;
import h5py
from tqdm import tqdm
import sys
import random
import os

import vispy.scene
from vispy.scene import visuals

from helena import globimap3d

#
# File Download Helper
#
class DownloadProgressBar(tqdm):
    def update_to(self, b=1, bsize=1, tsize=None):
        if tsize is not None:
            self.total = tsize
        self.update(b * bsize - self.n)

def download_url(url, output_path):
    print("Downloading %s" % (url))
    with DownloadProgressBar(unit='B', unit_scale=True,
                             miniters=1, desc=url.split('/')[-1]) as t:
        urllib.request.urlretrieve(url, filename=output_path, reporthook=t.update_to)



#
# Make a canvas and add simple view
#

scale = 100

def get_scaled(vertices, scale=10):
    # scale to unit cube
    vertices = vertices - np.min(vertices,axis=0)
    vertices = vertices / (np.max(vertices,axis=0) / scale)
    return (vertices)

#
# Download if needed
#
if not os.path.exists("modelnet10.h5"):
    download_url("https://api.bgd.ed.tum.de/datasets/pointclouds/modelnet10.h5","modelnet10.h5")


with h5py.File("modelnet10.h5","r") as f:
    groupnames = [x for x in f];
    choice = random.choice(groupnames)
    print(f"Choosing model number {choice}")
    vertices = get_scaled(f[choice]["coords"][:], scale)

#    
# Encode as globimap3d
#

m = globimap3d()
m.configure(5,18)
for v in vertices:
    m.put(v[0],v[1],v[2])
print(m.summary())

from itertools import product
vertices2 = np.array([a for a in product(range(scale),range(scale),range(scale)) if m.get(a[0],a[1],a[2])])
print(vertices2)
print("Original %d: Probabilistic: %d" % (len(vertices),len(vertices2)))

canvas = vispy.scene.SceneCanvas(title = choice, keys='interactive', show=True)
view = canvas.central_widget.add_view()

# create scatter object and fill in the data
scatter = visuals.Markers()
scatter.set_data(vertices2, edge_color=None, face_color=(1, 1, 1, .5), size=5)

view.add(scatter)

view.camera = 'arcball'  # or try 'turntable'

# add a colored 3D axis for orientation
axis = visuals.XYZAxis(parent=view.scene)

if __name__ == '__main__':
    import sys
    if sys.flags.interactive != 1:
        vispy.app.run()
        
