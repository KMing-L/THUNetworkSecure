from flask import Flask, flash, redirect, render_template, request, url_for
import sqlite3

# 连接数据库
def connect_db():
    db = sqlite3.connect('test.db')
    db.cursor().execute('CREATE TABLE IF NOT EXISTS comments '
                        '(id INTEGER PRIMARY KEY, '
                        'comment TEXT)')
    db.cursor().execute('CREATE TABLE IF NOT EXISTS users '
                        '(id INTEGER PRIMARY KEY, '
                        'username TEXT, '
                        'password TEXT)')
    db.commit()
    return db

# 添加评论
def add_comment(comment):
    db = connect_db()
    db.cursor().execute('INSERT INTO comments (comment) '
                        'VALUES (?)', (comment,))
    db.commit()

# 得到评论
def get_comments(search_query=None):
    db = connect_db()
    results = []
    get_all_query = 'SELECT comment FROM comments'
    for (comment,) in db.cursor().execute(get_all_query).fetchall():
        if search_query is None or search_query in comment:
            results.append(comment)
    return results

# 添加用户
def add_user(username, password):
    db = connect_db()
    db.cursor().execute('INSERT INTO users (username, password) '
                        'VALUES (?, ?)', (username, password))
    db.commit()

# 用户登录
def login_user(username, password):
    db = connect_db()
    get_user_query = f"SELECT * FROM users WHERE username = '{username}' AND password = '{password}'"
    user = db.cursor().execute(get_user_query).fetchone()
    return user is not None

# 启动flask
app = Flask(__name__)
app.secret_key = 'xxxxxxx'
@app.route('/', methods=['GET', 'POST'])
def index():
    if request.method == 'POST':
        add_comment(request.form['comment'])

    search_query = request.args.get('q')

    comments = get_comments(search_query)

    return render_template('index.html',
                           comments=comments,
                           search_query=search_query)

# 注册用户
@app.route('/register', methods=['POST'])
def register():
    add_user(request.form['username'], request.form['password'])
    flash('User registered')
    return redirect(url_for('index'))

@app.route('/login', methods=['POST'])
def login():
    if login_user(request.form['username'], request.form['password']):
        flash(f'User {request.form["username"]} logged in')
    else:
        flash('User not logged in')
    return redirect(url_for('index'))

@app.route('/sudo_login', methods=['POST'])
def sudo_login():
    if request.form['username'] == 'sudo' and request.form['password'] == 'sudo':
        response = redirect(url_for('transfer'))
        response.set_cookie('username', 'sudo')
        return response
    else:
        return redirect(url_for('index'))

@app.route('/transfer', methods=['GET', 'POST'])
def transfer():
    username = request.cookies.get('username', None)
    if username != 'sudo':
        return redirect(url_for('index'))

    if request.method == 'POST':
        flash(f'transfer {request.form["amount"]} to {request.form["username"]}')
        return redirect(url_for('transfer'))
    
    return render_template('transfer.html')