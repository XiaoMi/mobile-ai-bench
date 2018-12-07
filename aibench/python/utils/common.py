import os
import zipfile
import tarfile
from six.moves import urllib


def download_and_extract_dataset(url, download_dir):
    filename = url.split('/')[-1]
    file_path = os.path.join(download_dir, filename)
    if not os.path.exists(file_path):
        if not os.path.exists(download_dir):
            os.makedirs(download_dir)

        print("Downloading %s" % url)
        file_path, _ = urllib.request.urlretrieve(url, file_path)

        if file_path.endswith(".zip"):
            zipfile.ZipFile(file=file_path, mode="r").extractall(download_dir)
        elif file_path.endswith((".tar.gz", ".tgz")):
            tarfile.open(name=file_path, mode="r:gz").extractall(download_dir)

        print("Done extracted to %s" % download_dir)
    else:
        print("Data has already downloaded and extracted.")