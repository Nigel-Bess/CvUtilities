import requests
import time

# Class to interact with a coco-annotator API backend
class CocoApiClient:

    def login(self, username, password):
        res = requests.post(
            'http://localhost:5000/api/user/login',
            json={
                'username': username,
                'password': password
            },
            headers={
                'Content-Type': 'application/json'
            })
        self.login_cookie = res.cookies
        if "session" in self.login_cookie:
            print("Logged into coco-annotator as " + username)
        else:
            raise Exception("Could not log into localhost:5000 with user " + username)
        return self.login_cookie

    def delete_dataset_by_name(self, name):
        res = requests.get('http://localhost:5000/api/dataset',
            cookies=self.login_cookie,
            headers={
                'Content-Type': 'application/json',
            })
        for ds in res.json():
            if ds['name'] == name:
                self.delete_dataset(ds['id'])
                return True
        return False

    def create_dataset(self, dataset_name):
        res = requests.post('http://localhost:5000/api/dataset',
            cookies=self.login_cookie,
            headers={
                'Content-Type': 'application/json',
            },
            json={'name': dataset_name})
        return res.json()
    
    def delete_dataset(self, dataset_id):
        res = requests.delete('http://localhost:5000/api/dataset/' + str(dataset_id),
            cookies=self.login_cookie,
            headers={
                'Content-Type': 'application/json',
            })
        self.destroy_undos()

    def destroy_undos(self):
        res = requests.get('http://localhost:5000/api/undo/list',
            cookies=self.login_cookie,
            headers={
                'Content-Type': 'application/json',
            })
        for undo in res.json():
            del_res = requests.delete('http://localhost:5000/api/undo',
                cookies=self.login_cookie,
                headers={
                    'Content-Type': 'application/json',
                },
                json={"id": undo["id"], "instance": undo["instance"]})

    # Workaround for annotator not updating thumbnails upon import!
    def refresh_images(self, dataset, retries=10):
        dataset_id = dataset["id"]
        image_res = requests.get('http://localhost:5000/api/dataset/' + str(dataset_id) + "/coco",
            cookies=self.login_cookie,
            headers={
                'Content-Type': 'application/json',
            })
        images = image_res.json()
        if retries > 0 and len(images["images"]) == 0:
            time.sleep(1)
            return self.refresh_images(dataset, retries-1)
        
        # Query 1 more time now that indexing has begun
        time.sleep(5)
        image_res = requests.get('http://localhost:5000/api/dataset/' + str(dataset_id) + "/coco",
            cookies=self.login_cookie,
            headers={
                'Content-Type': 'application/json',
            })

        print("Redrawing " + str(len(images["images"])) + " labeled images")
        for img in images["images"]:
            inner = requests.post('http://localhost:5000/api/annotator/data',
                headers={
                    'Content-Type': 'application/json',
                },
                json={"dataset": dataset, "image": img},
                cookies=self.login_cookie)
    
    def upload_coco_file(self, file, dataset_id):
        # Always force a scan before an upload so images aren't missed
        requests.get('http://localhost:5000/api/dataset/' + str(dataset_id) + "/scan",
            cookies=self.login_cookie,
            headers={
                'Content-Type': 'application/json',
            })
        res = requests.post('http://localhost:5000/api/dataset/' + str(dataset_id) + '/coco',
            cookies=self.login_cookie,
            # Include multipart form post of the file_contents file
            files={'coco': file})
        return res.json()
        
