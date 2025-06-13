import os
import requests
import json

class CvatApiClient:

    #set up login for the cvat api
    def login(self, username, password):
        payload={
            'username': username,
            'password': password
        }
        response = requests.post('http://localhost:8080/api/auth/login', json=payload)
        response.raise_for_status()
        print("Status Code:", response.status_code)
        if response.status_code == 200:
            print("Login Successful !!")
        else:
            raise Exception("Login failed: {response.text}")

        return response.json()["key"]

    #get request to retrieve existing project info
    def get_projects(token):
        headers={
            "Authorization": f"Token {token}"
        } 
        response = requests.get('http://localhost:8080/api/projects', headers=headers)
        print("Status Code:", response.status_code)
        if response.status_code >= 200 and response.status_code < 300:
            projects = response.json()
            print("Projects: ",  projects)
            return projects
        else:
            print("Failed to get project details: {response.text}")
            return []

    #create projects by name 
    def create_project(self, token, project_name, labels, description): 
        project_data={
            'name': project_name,
            'labels': labels,
            'description': description
        }
        headers={
            "Authorization": f"Token {token}",
            'Content-Type': 'application/json'
        }
        response = requests.post('http://localhost:8080/api/projects', json=project_data, headers=headers)
        print("Status Code:", response.status_code)
        if response.status_code >= 200 and response.status_code < 300:
            projects = response.json()
            print("Projects: ",  projects)
            return projects
        else:
            print("Failed to create project: ", {response.text})
            return []

    #delete project by name
    def delete_project_by_name(token, project_name):
        headers={
            "Authorization": f"Token {token}"
        }  
        response = requests.get('http://localhost:8080/api/projects', headers=headers)
        print("Status Code:", response.status_code)
        if response.status_code >= 200 and response.status_code < 300:
            projects = response.json()
            print("Projects: ",  projects)

        else:
            print("Failed to get project details: {response.text}")
        projects = response.json().get("projects", [])
        project_id = None
        for project in projects:
            if project['name'] == project_name:
                project_id = project['id']
                break
        if not project_id:
            print("Project", project_name, "not found.")

        header={
            "Authorization": f"Token {token}"
            'Content-Type': 'application/json'
        }

        response = requests.delete('http://localhost:8080/api/projects/2', cookies=cookies, headers=header)

        if response.status_code >= 200 and response.status_code < 300:
            print("Project deleted successfully!!!")
        else:
            print("Failed to delete project!")
        return

    #get request to retrieve existing task info
    def list_tasks(token):
        headers={
            "Authorization": f"Token {token}"
        }

        response = requests.get('http://localhost:8080/api/tasks', headers=headers)
        print("Status Code:", response.status_code)
        if response.status_code >= 200 and response.status_code < 300:
            tasks = response.json()
            print("Tasks Info: ",  tasks)
            return tasks
        else:
            print("Failed to get task details: ", {response.text})
            return []

    #create task by id & name
    def create_task(self, token, project_id, task_name):
        task_data={
            'name': task_name,
            'project_id': project_id
        }
        header={
            "Authorization": f"Token {token}",
            'Content-Type': 'application/json'
        }
        response = requests.post('http://localhost:8080/api/tasks', json=task_data, headers=header)
        print("Status Code:", response.status_code)
        if response.status_code >= 200 and response.status_code < 300:
            tasks = response.json()
            print("Tasks: ", tasks)
        else:
            print("Failed to create task: ", {response.text})

    #Upload image data to task after creation
    def upload_data_to_task(self, token, task_id, image_directory):
        headers={
            "Authorization": f"Token {token}"
        }   

        url = f"http://localhost:8080/api/tasks/" + str(task_id) + "/data"
        files = [("client_files", (image_directory, open(image_directory, "rb"))) ]
        data = {
            "image_quality": 100,
            "data_type": "compressed",
            "use_zip_chunks": True
        }
        response = requests.post(url, headers=headers, files=files, data=data)
        response.raise_for_status()

        print("Status Code:", response.status_code)
        print("Response: ", response.text)
        if response.status_code >= 200 and response.status_code < 300:
            print("Successfully uploaded data to task" + str(task_id))
        else:
            print("Data upload failed for task" + str(task_id))
        
        task_url = f"http://localhost:8080/api/tasks/" + str(task_id)
        res = requests.get(task_url, headers=headers)
        print("Metadata Response: ", res.json())

    #Upload/create/delete annotations to the task 
    def upload_annotations_to_task(self, token, task_id, file_path, format_name):
        url = f"http://localhost:8080/api/tasks/" + str(task_id) + "/annotations"
        headers={
            "Authorization": f"Token {token}"
        }  
        files = {
            'annotation_file' : (os.path.basename(file_path), open(file_path, 'rb'))
        }
        data = {
            'format' : format_name,
            'action' : 'update' #alternate options are - create, delete
        }
        res = requests.put(url, headers=headers, files=files, data=data)
        res.raise_for_status()
        print("Status Code:", res.status_code)
        print("Response: ", res.text)
        if res.status_code >= 200 and res.status_code < 300:
            print("Annotations uploaded to task " + str(task_id)) 
        else:
            print("Annotation upload failed for task" + str(task_id))

        response = requests.get(url, headers=headers)
        print("Status Code:", response.status_code)
        print("Response: ", response.text)

        return res.json()

    #delete a project directly via Id
    def delete_project_by_id(self, token, project_id):
        url = f"http://localhost:8080/api/projects/" + str(project_id)
        headers={
            "Authorization": f"Token {token}"
        }  
        response = requests.delete(url, headers=headers)
        response.raise_for_status()
        if response.status_code >= 200 and response.status_code < 300:
            print("Project deleted successfully!!")
        else:
            print("Project deletion failed!!")

    #delete a task via Id
    def delete_task_by_id(self, token, task_id):
        url = f"http://localhost:8080/api/tasks/" + str(task_id)
        headers={
            "Authorization": f"Token {token}"
        }  
        response = requests.delete(url, headers=headers)
        response.raise_for_status()
        if response.status_code >= 200 and response.status_code < 300:
            print("Task deleted successfully!!")
        else:
            print("Task deletion failed!!")