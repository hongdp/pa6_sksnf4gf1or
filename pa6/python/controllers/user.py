import hashlib
import random
import string

from utils import *
from flask import *
import os
from flask_mail import Message

user = Blueprint('user', __name__, template_folder='views')
APP_ROOT = os.path.dirname(os.path.abspath(__file__))


@user.route(append_key('/user'), methods=['GET', 'POST'])
def reg():
    # redirect to user/edit
    if request.method == 'GET':
        if 'username' in session:
            return redirect(url_for('useredit.edit'))

    if request.method == 'POST':
        username = request.form['username']
        con = mysql.connection
        cur = con.cursor()
        cur.execute("SELECT * FROM User WHERE username='%s'" % (username))
        exist = cur.fetchall()
        if exist:
            error = 'username already exist'
            return render_template('user.html', error=error)

        if request.form['password'] != request.form['re-password']:
            error = 'password does not match'
            return render_template('user.html', error=error)
        hash_password = hashlib.sha224(request.form['password']).hexdigest()

        msg = Message('welcome!', recipients=[request.form['email']])
        msg.body = 'Congratulation! Your just started journey to the Great Album Wall! Please click the link below to jump on the trip!'
        home_url = request.base_url
        home_url = home_url[:-5]
        msg.body += home_url
        print request.base_url
        mail.send(msg)

        cur.execute("INSERT INTO User VALUES ('%s', '%s', '%s', '%s', '%s')" % (request.form['username'],
                                                                                request.form['firstname'],
                                                                                request.form['lastname'], hash_password,
                                                                                request.form['email']))
        session['username'] = request.form['username']
        renew_session(session)
        con.commit()
        return redirect(url_for('main.main_route'))

    return render_template('user.html')


@user.route(append_key('/user/delete'), methods=['POST'])
def delete_user():
    # redirect to user/edit
    # if request.method == 'GET':
    #   if 'username' in session:
    #       return redirect(url)
    if not session_exists(session):
        return render_template("noLogin.html", login=False), 403
    elif session_is_expired(session):
        session.clear()
        return render_template("sessionExpire.html", login=False)

    username = session['username']
    con = mysql.connection
    cur = con.cursor()
    cur.execute("SELECT * FROM User WHERE username='%s'" % (username))
    exist = cur.fetchall()
    if not exist:
        abort(404)
    cur.execute(
        "SELECT Photo.picid, Photo.url FROM User, Album, Contain, Photo WHERE User.username = Album.username AND Album.albumid = Contain.albumid AND Contain.picid = Photo.picid AND User.username = '%s'" % (
            username))
    pics = cur.fetchall()
    cur.execute(
        "DELETE FROM Photo WHERE Photo.picid IN (SELECT Contain.picid as picid FROM User, Album, Contain WHERE User.username = Album.username AND Album.albumid = Contain.albumid AND User.username = '%s')" % (
            username))
    con.commit()
    for pic in pics:
        url = '..' + pic[1]
        os.remove(os.path.join(APP_ROOT, url))
    cur.execute("DELETE FROM User WHERE username = '%s'" % (username))
    con.commit()
    session.clear()
    return redirect(url_for('main.main_route'))


@user.route(append_key('/user/forget'), methods=['POST', 'GET'])
def forget_password():
    if request.method == 'GET':
        return render_template("forget.html", exist=True)
    if request.method == 'POST':
        username = request.form['username']
        password = ''.join(random.choice(string.ascii_lowercase + string.digits) for _ in range(8))
        # print password
        hash_password = hashlib.sha224(password).hexdigest()
        con = mysql.connection
        cur = con.cursor()
        cur.execute("UPDATE User SET password='%s' WHERE username='%s'" % (hash_password, username))
        cur.execute("SELECT username, email FROM User WHERE username = '%s'" % username)
        msgs = cur.fetchall()
        if not msgs:
            return render_template("forget.html", exist=False)
        con.commit()

        msg = Message('Password Changed', recipients=[msgs[0][1]])
        msg.body = 'Your new password is %s' % password
        mail.send(msg)
        return redirect(url_for('login.login_func'))
