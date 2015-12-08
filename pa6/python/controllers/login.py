import hashlib

from utils import *
from flask import *

login = Blueprint('login', __name__, template_folder='views')


@login.route(append_key('/user/login'), methods=['GET', 'POST'])
def login_func():
    url = request.args.get('url')
    if request.method == 'GET':
        if session_exists(session):
            renew_session(session)
            if url:
                return redirect(url)
            else:
                return redirect(url_for('main.main_route'))
    if request.method == 'POST':
        username = request.form['username']
        hash_password = hashlib.sha224(request.form['password']).hexdigest()[0:20]
        con = mysql.connection
        cur = con.cursor()
        cur.execute("SELECT username, password FROM User WHERE username='%s'" % (username))
        userinfo = cur.fetchall()
        if not userinfo:
            error = 'Username does not exist'
            return render_template('login.html', error=error)

        if userinfo[0][1] != hash_password:
            error = 'password incorrent'
            return render_template('login.html', error=error)

        flash('You were successfully logged in')
        session['username'] = request.form['username']
        renew_session(session)
        if not url:
            return redirect(url_for('main.main_route'))
        return redirect(url)
    return render_template('login.html', hideLogin=True)


@login.route(append_key('/user/logout'), methods=['GET'])
def logout_func():
    session.clear()
    return redirect(url_for('main.main_route'))
