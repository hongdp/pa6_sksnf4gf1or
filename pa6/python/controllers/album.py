import hashlib

from utils import *
from flask import *
import os

album = Blueprint('album', __name__, template_folder='views')

ALLOWED_EXTENSIONS = {'png', 'jpg', 'bmp', 'gif'}
APP_ROOT = os.path.dirname(os.path.abspath(__file__))
UPLOAD_FOLDER = os.path.join(APP_ROOT, '../static/pictures')


def allowed_file(filename):
    lower_file_name = filename.lower()
    return '.' in lower_file_name and lower_file_name.rsplit('.', 1)[1] in ALLOWED_EXTENSIONS


@album.route(append_key('/album/edit'), methods=['GET', 'POST'])
def album_edit_route():
    error = ''
    album_id = request.args.get('id')
    if not album_id:
        abort(404)
    con = mysql.connection
    cur = con.cursor()
    cur.execute("SELECT albumid, username FROM Album WHERE albumid=%s" % (album_id))
    album = cur.fetchall()
    if not album:
        abort(404)

    # Authentication Codes
    login = False
    if session_exists(session):
        if session_is_expired(session):
            session.clear()
            return render_template('sessionExpire.html', login=False)
        else:
            login = True
            renew_session(session)
            if album[0][1] != session['username']:
                return render_template('noAccess.html', login=True), 403
    else:
        return render_template('noLogin.html', login=False), 403
    # Authentication Codes End

    # add picture to static/pictures
    if request.method == 'POST':
        # add picture to static/pictures
        if request.form['op'] == 'add':
            file = ""
            if "file" in request.files:
                file = request.files['file']

            if file and allowed_file(file.filename):
                format = file.filename.rsplit('.', 1)[1]
                date = time.strftime('%Y-%m-%d', time.gmtime())
                curtime = time.strftime('%Y-%m-%d-%H-%M-%S', time.gmtime())
                pic_id = hashlib.sha224(file.filename + curtime).hexdigest()
                picname = pic_id + '.' + format
                url = '/static/pictures/' + picname
                if not os.path.exists(UPLOAD_FOLDER):
                    os.makedirs(UPLOAD_FOLDER)
                file.save(os.path.join(UPLOAD_FOLDER, picname))

                seqnum = 1
                sqlphoto = "INSERT INTO Photo (picid, url, format, date) VALUES ('%s', '%s', '%s', '%s')" % (
                    pic_id, url, format, date)
                cur.execute(sqlphoto)
                con.commit()

                cur.execute("SELECT MAX(sequencenum) FROM Contain WHERE Contain.albumid = %s" % (album_id))
                maxseq = cur.fetchall()[0][0]
                if maxseq:
                    seqnum = maxseq + 1

                sqlcontain = "INSERT INTO Contain (albumid, picid, caption, sequencenum) VALUES(%s, '%s', '', %d )" % (
                    album_id, pic_id, seqnum)
                cur.execute(sqlcontain)
                con.commit()

        if request.form['op'] == 'delete':
            pic_id = ""
            if "picid" in request.form:
                pic_id = request.form['picid']
            sqlcontain = "DELETE FROM Contain WHERE Contain.picid = '%s'" % (pic_id)
            cur.execute(sqlcontain)
            con.commit()

            sqlphoto = "SELECT url FROM Photo WHERE Photo.picid = '%s'" % (pic_id)
            cur.execute(sqlphoto)
            msgsurl = cur.fetchall()
            url = msgsurl[0][0]

            sqlphoto = "DELETE FROM Photo WHERE Photo.picid = '%s'" % (pic_id)
            cur.execute(sqlphoto)
            con.commit()

            url = ".." + url
            os.remove(os.path.join(APP_ROOT, url))

        if request.form['op'] == 'modifyAlbumName':
            new_album_name = ''
            if "albumName" in request.form:
                new_album_name = request.form['albumName']
            sql_update_album_name = "UPDATE Album SET title = '%s' WHERE albumid = %s" % (new_album_name, album_id)
            cur.execute(sql_update_album_name)
            con.commit()

        if request.form['op'] == 'revoke':
            username = ""
            if "username" in request.form:
                username = request.form['username']
            sql_revoke = "DELETE FROM AlbumAccess WHERE username = '%s' AND albumid = '%s'" % (username, album_id)
            cur.execute(sql_revoke)
            con.commit()

        if request.form['op'] == 'modifyAccess':
            privacy = ""
            if "privacy" in request.form:
                privacy = request.form['privacy']
            if privacy == 'private' or privacy == 'public':
                sql_revoke = "UPDATE Album SET access = '%s' WHERE albumid = '%s'" % (privacy, album_id)
                cur.execute(sql_revoke)
                if privacy == 'public':
                    sql_delete_access = "DELETE FROM AlbumAccess WHERE albumid = %s" % (album_id)
                    cur.execute(sql_delete_access)
                con.commit()

        if request.form['op'] == 'addAccess':
            username = ""
            if "username" in request.form:
                username = request.form['username']
            find_user = "SELECT * FROM User WHERE username = '%s'" % (username)
            cur.execute(find_user)
            user = cur.fetchall()
            if not user:
                error = 'No such a user.'
            else:
                findAccess = "SELECT * FROM AlbumAccess WHERE username = '%s' AND albumid = %s" % (username, album_id)
                cur.execute(findAccess)
                access = cur.fetchall()
                if access:
                    error = 'User already has authentication'
                else:
                    sqladd = "INSERT INTO AlbumAccess(albumid, username) Values (%s, '%s')" % (album_id, username)
                    cur.execute(sqladd)
                    con.commit()

    cur.execute(
        "SELECT Photo.picid, url, Contain.caption, date FROM Photo, Contain WHERE Photo.picid = Contain.picid AND Contain.albumid = '%s' ORDER BY sequencenum " % (
            album_id))

    photos = cur.fetchall()
    cur.execute("SELECT username, title, access FROM Album WHERE albumid = %s" % (album_id))
    album_info = cur.fetchall()
    cur.execute("SELECT username FROM AlbumAccess WHERE albumid = %s" % (album_id))
    access_users = cur.fetchall()
    options = {
        "edit": True,
        "photos": photos,
        "username": album_info[0][0],
        "albumname": album_info[0][1],
        "privacy": album_info[0][2],
        "albumid": album_id,
        "login": login,
        "error": error,
        "accessUsers": access_users
    }
    return render_template("album.html", **options)


@album.route(append_key('/album'), methods=['GET'])
def album_route():
    album_id = request.args.get('id')
    if not album_id:
        abort(404)
    cur = mysql.connection.cursor()
    cur.execute("SELECT albumid, username, access FROM Album WHERE albumid=%s" % (album_id))
    album = cur.fetchall()
    if not album:
        abort(404)

    # Authentication Codes
    login = False
    if album[0][2] == 'private':
        if session_exists(session):
            if session_is_expired(session):
                session.clear()
                return render_template('sessionExpire.html', login=False)
            else:
                login = True
                if album[0][1] == session['username']:
                    renew_session(session)
                else:
                    cur.execute("SELECT username FROM AlbumAccess WHERE albumid=%s and username='%s'" % (
                        album_id, session['username']))
                    authUser = cur.fetchall()
                    renew_session(session)
                    if not authUser:
                        return render_template('noAccess.html', login=True), 403
        else:
            return render_template('noLogin.html', login=False), 403
    else:
        if session_exists(session):
            if session_is_expired(session):
                print 'session expired'
                session.clear()
            else:
                login = True
                renew_session(session)
                # Authentication Codes End

    cur.execute(
        "SELECT Photo.picid, url, Contain.caption, date FROM Photo, Contain WHERE Photo.picid = Contain.picid AND Contain.albumid = '%s' ORDER BY sequencenum " % (
            album_id))
    photos = cur.fetchall()
    cur.execute("SELECT username, title FROM Album WHERE albumid = '%s'" % (album_id))
    album_info = cur.fetchall()
    options = {
        "edit": False,
        "photos": photos,
        "username": album_info[0][0],
        "albumname": album_info[0][1],
        "albumid": album_id,
        "login": login
    }
    # print options
    return render_template("album.html", **options)
