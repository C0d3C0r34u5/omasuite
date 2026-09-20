# OmaSuite

An all-in-one mail, calendar, contacts and tasks suite for [Omarchy](https://omarchy.org),
built with Qt Quick (QML) and C++.

![OmaSuite](icons/omasuite.svg)

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

---

## Compiling and running

### Dependencies

- `qt6-base` (provides `qmake6`, `moc`)
- `qt6-declarative` (QML, Qt Quick, Qt Quick Controls 2)
- `qt6-networkauth` (OAuth2)
- `openssl` (TLS)
- `libsecret` (keyring storage)
- A C++17 compiler (`gcc` / `clang`) and `make`

### On Arch / Omarchy (recommended)

From the project directory, build and install the package:

```bash
makepkg -si
```

This produces and installs an `omasuite` package. Launch it from your app menu,
or run:

```bash
omasuite
```

### Manual developer build

Build into `./build` and install to `~/.local`:

```bash
./install.sh
```

Or, without installing, just compile and run from the build directory:

```bash
mkdir -p build && cd build
qmake6 ../OmaSuite.pro
make -j"$(nproc)"
./omasuite
```

### First run

On first launch, OmaSuite shows a setup wizard. Add an account using either an
**app password** or **OAuth2** (see below). You can add more accounts later from
the "Add account" button in the sidebar.

Data is stored in `~/.local/share/Omarchy/OmaSuite/`; passwords and OAuth tokens
live in your system keyring, not on disk in plaintext.

---

## Setting up accounts

There are two ways to authenticate:

1. **App password (password)** — the simplest option. Generate an
   "app-specific password" from your provider and paste it into the wizard.
   Works for Gmail, Yahoo, Apple/iCloud, and any IMAP/SMTP server. **No client
   ID needed.**

2. **OAuth2 (browser)** — signs you in through your provider's login page and
   consent screen (no password is given to OmaSuite). This requires a
   **`client_id`** that you register with the provider (see the next section).

> **Apple/iCloud** does not offer public OAuth2 for mail/CalDAV/CardDAV. Use an
> [app-specific password](https://support.apple.com/HT204397) with the
> "App password" option.

---

## Getting your own OAuth `client_id`

When you use the OAuth2 option, your browser opens the provider's login page,
you sign in, and the provider asks your permission to grant OmaSuite access.
**That consent screen cannot appear until the app identifies itself with a
`client_id`** — a string the provider issues when *you* register an application
in their developer portal.

Because OmaSuite is open source and ships no secret, each user (or the project
maintainer) registers their own client. There is **no charge** to register a
client and use it in *testing mode* for yourself (up to ~100 test users per
provider). OmaSuite uses **PKCE** with a loopback redirect
(`http://localhost`, any port), so **no client secret is required**.

Here's how to register a client for each provider.

### Gmail (Google)

1. Open the [Google Cloud Console](https://console.cloud.google.com/).
2. Create a **project** (top-left project dropdown → *New Project*) and select it.
3. Enable the required APIs (search for each, then click *Enable*):
   - **Gmail API**
   - **Google Calendar API**
   - **People API** (this serves the contacts scope)
4. Open **APIs & Services → OAuth consent screen** and configure:
   - User type: **External**
   - App name: `OmaSuite`, plus your support/developer email.
   - Add scopes: `https://mail.google.com/`, `https://www.googleapis.com/auth/calendar`,
     `https://www.googleapis.com/auth/contacts`.
   - Under **Test users**, add your own Google account (and any friends you want
     to grant access). In testing mode only test users can sign in.
5. Open **APIs & Services → Credentials → Create Credentials → OAuth client ID**.
6. Application type: **Desktop app**, name it `OmaSuite`, and create it.
7. Copy the resulting **Client ID**, e.g.
   `1234567890-xxxxxxxxxxxx.apps.googleusercontent.com`.
8. Paste it into the OmaSuite wizard's **"OAuth client ID"** field when adding
   the Gmail account (choose the OAuth2 option).

> While in testing mode the consent screen shows an "unverified app" warning;
> test users click *Advanced → Go to OmaSuite (unsafe)* once. Removing that
> warning for the general public requires Google's app verification, and the
> Gmail (restricted) scope additionally requires a paid security assessment.

### Outlook / Microsoft 365

1. Open the [Azure portal → App registrations](https://portal.azure.com/#view/Microsoft_AAD_RegisteredApps).
2. Click **New registration**:
   - Name: `OmaSuite`
   - Supported account types: **"Accounts in any organizational directory and
     personal Microsoft accounts"** (or *Personal Microsoft accounts only* for
     just outlook.com).
   - Redirect URI: platform **Mobile and desktop applications**, value
     `http://localhost`. *(OmaSuite listens on a random local port, and the
     bare `http://localhost` registration covers any port.)*
3. Click **Register**.
4. Copy the **Application (client) ID**.
5. *(Optional but recommended)* Under **API permissions**, add the delegated
   permissions you plan to use, e.g. Microsoft Graph `Mail.ReadWrite`,
   `Calendars.ReadWrite`, `Contacts.ReadWrite`, `Tasks.ReadWrite`, and — for
   IMAP/SMTP/EWS OAuth — the "Office 365 Exchange Online" permissions
   `IMAP.AccessAsUser.All`, `SMTP.Send`, `EWS.AccessAsUser.All`.
6. Paste the client ID into the wizard when adding the Outlook/Exchange account.

> Microsoft has no free-form "test user" list like Google; unverified personal
> apps can sign in without admin consent, while work/school tenants may require
> admin consent depending on policy.

### Yahoo

1. Open the [Yahoo Developer Network](https://developer.yahoo.com/) and create
   an **app** for "Server-side / Client-side apps".
2. Set the redirect URI to `http://localhost` (or your loopback URL).
3. Choose the mail permissions you need (e.g. *Read/Write Mail*).
4. Copy the **Client ID**.

> **Note:** Yahoo's OAuth2 implementation requires a **client secret** in
> addition to the client ID (unlike Google/Microsoft PKCE-only desktop clients).
> OmaSuite's OAuth2 flow currently uses PKCE without a secret; Yahoo OAuth is
> therefore the least-tested path. If it doesn't sign in, use a Yahoo
> **app password** instead — that works without any registration.

---

## License

[GPL-3.0-or-later](LICENSE)
