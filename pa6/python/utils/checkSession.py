import time


def session_exists(session):
    if 'username' in session and 'lastActivity' in session:
        return True
    return False


def session_is_valid(session):
    if 'username' in session and not session_is_expired(session):
        return True
    return False


def renew_session(session):
    session['lastActivity'] = int(time.time())


def session_is_expired(session):
    currentTime = int(time.time())
    if 'lastActivity' in session:
        if currentTime - session['lastActivity'] <= 300:
            # print currentTime - session['lastActivity']
            return False
    return True


def get_user_info_from_session(session, mysql):
    if 'username' in session:
        cur = mysql.connection.cursor()
        cur.execute("SELECT * FROM User WHERE User.username = '%s'" % (session['username']))
        msgs = cur.fetchall()
        if msgs:
            return msgs
    return [[]]


def check_accessibility_of_session(session, mysql, albumid):
    if 'username' in session:
        cur = mysql.connection.cursor()
        cur.execute(
            "SELECT * FROM AlbumAccess WHERE username = '%s' AND albumid = '%s'" % (session['username'], albumid))
        msgs = cur.fetchall()
        if msgs:
            return True
    return False
