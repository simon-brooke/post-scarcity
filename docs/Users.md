# Users

I'm not yet sure what sort of objects users are. They may just be lists, interned in a special namespace such as *system.users*. They may be special purpose [[vector space]] objects (although I don't see why, apart from to get a special tag, which might be useful).

Every user object must contain credentials, and the credentials must be readable by system only; the credentials are either a hashed password or a cryptographic public key. The user object must also have an identifying name, and probably other identifying information. But it's not necessarily the case that every user on the system needs to be able to see the names of every other user on the system, so the identifying information (or the user object itself) may have [[access control]] lists.

There is a problem here with the principle of [[immutability]]; if an access control list on an object _foo_ contains a pointer to my user object so that I can read _foo_, and I change my password, then the immutability rule says that a new copy of the *system.users* namespace is created with a new copy of my user object. This new user object isn't on any access control list so by changing my password I actually can't read anything.

This means that what we put on access control lists is not user objects, but symbols (usernames) which are bound in *system.users* to user objects; the user object then needs a back-pointer to that username. A user group then becomes a list not of user objects but of interned user names.