# OmaSuite

An all-in-one mail, calendar, contacts and tasks suite for [Omarchy](https://omarchy.org),
built with Qt Quick (QML) and C++.

![Screenshot](icons/omasuite.svg)

## Features

- **Mail** — IMAP / SMTP (SSL/TLS) with message list, reading pane and compose.
- **Calendar** — CalDAV with a month view and event management.
- **Contacts** — CardDAV with search.
- **Tasks** — CalDAV VTODO with pending/done filtering.
- **Accounts** — first-run setup wizard for Gmail, Outlook, Yahoo, Apple/iCloud,
  Microsoft Exchange, or any manual IMAP/SMTP/CalDAV/CardDAV server.
- **Auth** — app passwords *and* OAuth2 (PKCE) where supported, plus EWS (SOAP)
  for Microsoft Exchange Online.
- **Security** — passwords and OAuth tokens are stored in the Secret Service
  (GNOME Keyring / KWallet) via libsecret, never in plaintext. TLS certificates
  are verified by default.

## Building

### From the AUR / source tarball

```bash
makepkg -si
```

### Manually (developer build, installs to `~/.local`)

```bash
./install.sh
```

Requires: `qt6-base`, `qt6-declarative`, `qt6-networkauth`, `openssl`, `libsecret`,
plus a C++17 compiler and `qmake6`.

## OAuth2 and the `client_id`

When you pick a Google account and choose "Sign in with OAuth2", OmaSuite opens
your browser to Google's login page where you type your email and password, and
Google asks your permission to allow the app to access your mail, calendar and
contacts. **That part is exactly what you described.**

However, before that consent screen can appear, the app must identify itself to
Google with a **`client_id`**. Google issues a `client_id` when someone registers
the application in the [Google Cloud Console](https://console.cloud.google.com/).
There is no universal, built-in `client_id` that works for everyone — it is unique
to the registered app. The same applies to Microsoft and Yahoo.

Because OmaSuite is open source and can't ship a secret tied to a specific
registration, you have two options:

1. **Ship a shared `client_id` (recommended for the project maintainer).**
   Register a "Desktop app" OAuth client for your provider, then set the
   `oauthClientId` field in `src/account/provider.cpp` (or enter it once in the
   setup wizard's "OAuth client ID" field). End users then only ever see the
   browser consent screen — no ID entry required. Use a loopback redirect URI
   (e.g. `http://127.0.0.1`) and PKCE, so **no client secret is needed**.

2. **Let each user register their own app** and paste their `client_id` into the
   setup wizard. More friction, but zero trust in a shared registration.

### Registering a Google "Desktop app" client

1. Go to the [Google Cloud Console](https://console.cloud.google.com/apis/credentials).
2. Create a project, then **Create Credentials → OAuth client ID → Desktop app**.
3. Copy the generated `client_id` (e.g. `1234567890-xxxx.apps.googleusercontent.com`).
4. Enter it in the OmaSuite setup wizard (or bake it into `provider.cpp`).

For Microsoft, create an "Mobile and desktop applications" registration in
[Azure App registrations](https://portal.azure.com/#view/Microsoft_AAD_RegisteredApps)
and set a loopback redirect URI.

> **Apple/iCloud** does not provide public OAuth2 for mail/CalDAV/CardDAV. Use an
> [app-specific password](https://support.apple.com/HT204397) with the
> "App password" option instead.

## License

[GPL-3.0-or-later](LICENSE)
