from utils import *
from flask import *
import os

albums = Blueprint('albums', __name__, template_folder='views')
APP_ROOT = os.path.dirname(os.path.abspath(__file__))


@albums.route(append_key('/albums/edit'), methods=['GET', 'POST'])
def albums_edit_route():
    # Authentication Codes
    login = False
    if session_exists(session):
        if session_is_expired(session):
            session.clear()
            return render_template('sessionExpire.html', login=login)
        else:
            username = session['username']
    else:
        return render_template('noLogin.html', login=login), 403
    if session_exists(session):
        login = True
        renew_session(session)
    # Authentication Codes End
    con = mysql.connection
    cur = con.cursor()

    if request.method == 'POST':
        if request.form['op'] == 'delete':
            album_id = request.form['albumid']
            sql_get_picids = "SELECT picid FROM Contain WHERE Contain.albumid = %s" % album_id
            cur.execute(sql_get_picids)
            picids = cur.fetchall()
            cur.execute("DELETE FROM Contain WHERE Contain.albumid = %s" % album_id)
            for picid_row in picids:
                picid = picid_row[0]
                sql_pic_from_contain = "DELETE FROM Contain WHERE Contain.picid = '%s'" % picid
                cur.execute(sql_pic_from_contain)
                con.commit()

                sql_url_from_photo = "SELECT url FROM Photo WHERE Photo.picid = '%s'" % picid
                cur.execute(sql_url_from_photo)
                msgsurl = cur.fetchall()
                url = msgsurl[0][0]

                sql_delete_photo = "DELETE FROM Photo WHERE Photo.picid = '%s'" % picid
                cur.execute(sql_delete_photo)
                con.commit()

                url = ".." + url
                os.remove(os.path.join(APP_ROOT, url))
            cur.execute("DELETE FROM Album WHERE albumid = '%s'" % album_id)

        if request.form['op'] == 'add':
            title = request.form['title']
            date = time.strftime('%Y-%m-%d', time.gmtime())
            sqlcode = "INSERT INTO Album (title, created, lastupdated, username, access) \
            VALUES ('%s', '%s', '%s', '%s', 'private')" % (title, date, date, username)
            cur.execute(sqlcode)
            cur.execute("SELECT LAST_INSERT_ID()")
            id = cur.fetchall()
            con.commit()

    cur.execute("SELECT * FROM Album WHERE username ='%s'" % (username))
    msgs = cur.fetchall()
    con.commit()
    options = {
        "edit": True
    }
    return render_template("albums.html", albums=msgs, username=username, login=login, **options)


@albums.route(append_key('/albums'), methods=['GET'])
def albums_route():
    username = ''
    con = mysql.connection
    cur = con.cursor()
    msgs = {}

    # Authentication Codes
    if session_is_valid(session):
        username = session['username']
        renew_session(session)
        cur.execute("SELECT * FROM Album WHERE username='%s'" % (username))
        msgs = cur.fetchall()
        options = {
            "edit": False,
            "login": True
        }
        return render_template("albums.html", username=username, albums=msgs, **options)
    elif session_is_expired(session):
        session.clear()
    # Authentication Codes End

    cur.execute("SELECT * FROM Album WHERE access ='public'")
    msgs = cur.fetchall()
    options = {
        "edit": False
    }
    return render_template("albums.html", username=username, albums=msgs, **options)
