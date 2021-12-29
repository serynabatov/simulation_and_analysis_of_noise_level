from flask import Flask, request

app = Flask(__name__)

arr = []

@app.route('/', methods=['POST'])
def hello_world():  # put application's code here
    print(request.data.decode('utf-8'))
    return 'aa'


if __name__ == '__main__':
    app.run(debug=True)
