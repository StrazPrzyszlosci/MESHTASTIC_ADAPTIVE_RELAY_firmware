# Formularz betatestowy - Adaptive Relay Firmware (v2.8.2-ar.3)

**[PL (ta sekcja)](#formularz-betatestowy--adaptive-relay-firmware-v282-ar2) · [EN (English)](#beta-test-form--adaptive-relay-firmware-v282-ar2)**

Dziękujemy za testowanie eksperymentalnego firmware Adaptive Relay
(N3 census + SHEP2D rescue + CEF_BOOST). Wypełnij ten formularz po
minimum **24 h** pracy urządzenia w Twojej sieci. Skopiuj treść,
zamień `[ ]` na `[x]` przy wybranych odpowiedziach i wyślij jako
**GitHub issue** w tym repozytorium (albo bezpośrednio do maintainera).

Testowany firmware: release **v2.8.2-ar.3** z
<https://github.com/StrazPrzyszlosci/MESHTASTIC_ADAPTIVE_RELAY_firmware/releases/tag/v2.8.2-ar.3>

---

## 1. Informacje o teście

- Urządzenie (model): \_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_
- Wariant wgranego firmware:
  - [ ] zwykły (quiet, np. `firmware-…-2.8.2-ar.uf2/.bin`)
  - [ ] `-log` (diagnostyczny, np. `firmware-…-2.8.2-ar-log.uf2/.bin`)
- Metoda wgrywania:
  - [ ] `.uf2` (double-tap reset → drag & drop)
  - [ ] `.zip` (Web Flasher / aktualizacja BLE)
  - [ ] `.bin` przez esptool/PlatformIO (Heltec)
  - [ ] inna: \_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_
- Czy przed wgraniem zrobiłeś/-aś backup konfiguracji? [ ] TAK [ ] NIE
- Region i ustawienia LoRa (region, preset, slot czasowy): \_\_\_\_\_\_\_\_
- Ile nodów ma Twoja sieć (w zasięgu, łącznie z tym urządzeniem): \_\_\_\_
- Ile nodów w Twojej sieci ma **stockowy** firmware Meshtastic: \_\_\_\_
- Ile nodów ma firmware **Adaptive Relay** (ten eksperymentalny): \_\_\_\_
- Rola testowanego urządzenia:
  - [ ] router/repeater (zawsze zasilany, stała pozycja)
  - [ ] node mobilny/przenośny
  - [ ] tracker/sensor
- Data wgrania: \_\_\_\_\_\_\_ · Data wypełnienia: \_\_\_\_\_\_\_
- Czas pracy urządzenia na tym firmware przed wypełnieniem: \_\_\_\_ h

---

## 2. Instalacja i uruchamianie

**A1. Czy firmware się wgrywa bez błędów?**

- [ ] TAK
- [ ] NIE - opisz błąd/komunikat: \_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_

**A2. Czy urządzenie się bootoje poprawnie (uruchamia się za każdym razem, do ok. 60 s po włączeniu zasilania)?**

- [ ] TAK - zawsze
- [ ] CZĘŚCIOWO - zdarza się, że nie startuje (jak często?: \_\_\_\_)
- [ ] NIE - nie uruchamia się wcale
- [ ] NIE TESTOWAŁEM

**A3. Czy po restarcie zasilania konfiguracja jest zachowana (kanały, nazwa, klucze)?**

- [ ] TAK [ ] NIE [ ] CZĘŚCIOWO [ ] NIE TESTOWAŁEM

**A4. Czy po downgrade'zie/powrocie na oficjalny firmware urządzenie działa normalnie?**

- [ ] TAK [ ] NIE [ ] NIE WRACAŁEM NA OFICJALNY [ ] NIE TESTOWAŁEM

---

## 3. Łączność z telefonem / komputerem

**B1. Czy urządzenie łączy się z telefonem przez Bluetooth?**

- [ ] TAK - za pierwszym razem, stabilnie
- [ ] CZĘŚCIOWO - łączy się, ale z problemami (opisz: opóźnienia, rozłączenia, trzeba restartować BT)
- [ ] NIE - nie łączy się wcale
- [ ] NIE TESTOWAŁEM

**B2. Czy urządzenie „odczytuje się" w aplikacji (widać je na liście, aplikacja pobiera NodeInfo - nazwa, pozycja, bateria, telemetria)?**

- [ ] TAK [ ] CZĘŚCIOWO [ ] NIE [ ] NIE TESTOWAŁEM

**B3. Czy połączenie po USB/serial działa (aplikacja, `meshtastic` CLI, logowanie)?**

- [ ] TAK [ ] CZĘŚCIOWO [ ] NIE [ ] NIE TESTOWAŁEM

---

## 4. Test bazowy - sieć BEZ tego eksperymentalnego firmware

_Pozwala wykluczyć, że problemy z odbiorem pochodzą z reszty sieci, a nie z testowanego firmware. Jeśli możesz, wyłącz na czas testu pozostałe węzły z firmware Adaptive Relay._

**C1. Czy urządzenie odbiera wiadomości tekstowe od innych nodów, które NIE mają tego eksperymentalnego firmware (stockowe nadajniki)?**

- [ ] TAK - wszystkie/dostrzeżone na czas
- [ ] CZĘŚCIOWO - część wiadomości nie dochodzi (szacuj %: \_\_\_)
- [ ] NIE - brak odbioru
- [ ] NIE TESTOWAŁEM (cała sieć jest na firmware AR)

**C2. Czy w aplikacji/device widzisz inne nody na liście (NodeDB się zapełnia, SNR/RSSI są realistyczne)?**

- [ ] TAK [ ] CZĘŚCIOWO [ ] NIE [ ] NIE TESTOWAŁEM

**C3. Czy od stockowych nodów przychodzą pozycje i/lub telemetria?**

- [ ] TAK [ ] CZĘŚCIOWO [ ] NIE [ ] NIE TESTOWAŁEM

---

## 5. Komunikacja dwukierunkowa (firmware Adaptive Relay)

**D1. Czy urządzenie wysyła wiadomości tekstowe (z telefonu do innych nodów)?**

- [ ] TAK [ ] CZĘŚCIOWO [ ] NIE [ ] NIE TESTOWAŁEM

**D2. Czy urządzenie odbiera wiadomości tekstowe (także od nodów na firmware Adaptive Relay)?**

- [ ] TAK [ ] CZĘŚCIOWO [ ] NIE [ ] NIE TESTOWAŁEM

**D3. Czy potwierdzenia doręczenia działają (w aplikacji widać status „dostarczono/odczytano" - ACKi)?**

- [ ] TAK [ ] CZĘŚCIOWO [ ] NIE [ ] NIE TESTOWAŁEM

**D4. Test round-trip: wysłałeś/-aś wiadomość i dostałeś/-aś odpowiedź?**

- [ ] TAK [ ] NIE [ ] NIE TESTOWAŁEM

---

## 6. Warstwa Adaptive Relay (dotyczy roli routera i wariantu `-log`)

**E1. Czy urządzenie retransmituje pakiety (sąsiedzi/dalekie nody dostają wiadomości przez ten węzeł)?**

- [ ] TAK [ ] CZĘŚCIOWO [ ] NIE [ ] NIE WIEM / NIE UMIAŁEM SPRAWDZIĆ

**E2. (Tylko wariant `-log`) Czy w logu szeregowym pojawia się banner `AR: Adaptive Relay N3 diagnostic build …` oraz linie `AR-N3`/`AR-DUPE`/`AR-SHEP`/`AR-CB`?**

- [ ] TAK [ ] CZĘŚCIOWO (brakuje niektórych typów linii) [ ] NIE [ ] NIE TESTOWAŁEM

**E3. (Tylko wariant `-log`) Czy linia `S:B:…` wskazuje repo `StrazPrzyszlosci/MESHTASTIC_ADAPTIVE_RELAY_firmware`?**

- [ ] TAK [ ] NIE [ ] NIE TESTOWAŁEM

**E4. (Tylko wariant `-log`) Czy poza liniami `AR-`/`S:B:` log jest faktycznie „cichy" (mało wpisów telemetrii/ekranu/NodeDB)?**

- [ ] TAK [ ] NIE (za dużo szumu - przykłady?: \_\_\_\_)

---

## 7. Stabilność

**F1. Czy urządzenie pracuje bez nieplanowanych restartów?**

- [ ] TAK - 0 restartów przez czas testu
- [ ] CZĘŚCIOWO - sporadyczne reboots (ile razy?: \_\_\_)
- [ ] NIE - regularne zawieszki/restarty
- [ ] NIE WIEM

**F2. Czy urządzenie wraca do pracy po długim bezczynności/nocy (bez ręcznego restartu)?**

- [ ] TAK [ ] NIE [ ] NIE TESTOWAŁEM

**F3. Zużycie baterii względem oficjalnego firmware (jeśli zasilane z baterii):**

- [ ] BRAK RÓŻNICY [ ] GORZEJ [ ] LEPIEJ [ ] NIE TESTOWAŁEM

---

## 8. Opis obserwacji (dowolna forma)

- Anomalie, błędy, nietypowe zachowanie: \_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_
- \_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_
- \_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_

**Załączniki (mocno pożądane):**

- [ ] wycinek logu z liniami `AR-`/`S:B:` (dla wariantu `-log`:
      `grep -E "AR-|S:B:"` z zapisu serial)
- [ ] zrzut ekranu aplikacji z listą nodów
- [ ] export konfiguracji (bez kluczy!)

---

_Dzieląc się wynikami testów, potwierdzasz, że firmware jest eksperymentalny
i nie jest powiązany ani rekomendowany przez projekt Meshtastic. Nie wgrywaj
go na urządzenia, od których zależy Twoje bezpieczeństwo._

---

# Beta test form - Adaptive Relay Firmware (v2.8.2-ar.3)

**[PL (polski)](#formularz-betatestowy--adaptive-relay-firmware-v282-ar2) · [EN (this section)](#beta-test-form--adaptive-relay-firmware-v282-ar2)**

Thank you for testing the experimental Adaptive Relay firmware
(N3 census + SHEP2D rescue + CEF_BOOST). Please fill in this form after
at least **24 h** of device operation in your network. Copy the content,
change `[ ]` to `[x]` for your answers and submit it as a
**GitHub issue** in this repository (or send it directly to the maintainer).

Tested firmware: release **v2.8.2-ar.3** from
<https://github.com/StrazPrzyszlosci/MESHTASTIC_ADAPTIVE_RELAY_firmware/releases/tag/v2.8.2-ar.3>

---

## 1. Test setup

- Device (model): \_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_
- Flashed firmware variant:
  - [ ] regular (quiet, e.g. `firmware-…-2.8.2-ar.uf2/.bin`)
  - [ ] `-log` (diagnostic, e.g. `firmware-…-2.8.2-ar-log.uf2/.bin`)
- Flashing method:
  - [ ] `.uf2` (double-tap reset → drag & drop)
  - [ ] `.zip` (Web Flasher / BLE firmware update)
  - [ ] `.bin` via esptool/PlatformIO (Heltec)
  - [ ] other: \_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_
- Did you back up your configuration before flashing? [ ] YES [ ] NO
- LoRa region and settings (region, preset, time slot): \_\_\_\_\_\_\_\_
- How many nodes are in your network (in range, incl. this device): \_\_\_\_
- How many nodes in your network run **stock** Meshtastic firmware: \_\_\_\_
- How many nodes run the **Adaptive Relay** (this experimental) firmware: \_\_\_\_
- Role of the tested device:
  - [ ] router/repeater (always powered, fixed position)
  - [ ] mobile/portable node
  - [ ] tracker/sensor
- Flash date: \_\_\_\_\_\_\_ · Form filled on: \_\_\_\_\_\_\_
- Device uptime on this firmware before filling in: \_\_\_\_ h

---

## 2. Installation and boot

**A1. Does the firmware flash without errors?**

- [ ] YES
- [ ] NO - describe the error/message: \_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_

**A2. Does the device boot correctly (starts every time, within ~60 s of power-on)?**

- [ ] YES - always
- [ ] PARTIALLY - sometimes fails to start (how often?: \_\_\_\_)
- [ ] NO - does not start at all
- [ ] NOT TESTED

**A3. Is the configuration preserved across power cycles (channels, name, keys)?**

- [ ] YES [ ] NO [ ] PARTIALLY [ ] NOT TESTED

**A4. After downgrading back to the official firmware, does the device work normally?**

- [ ] YES [ ] NO [ ] DID NOT DOWNGRADE [ ] NOT TESTED

---

## 3. Phone / computer connectivity

**B1. Does the device connect to the phone over Bluetooth?**

- [ ] YES - first time, stable
- [ ] PARTIALLY - connects but with problems (describe: delays, drops, needs BT restart)
- [ ] NO - does not connect at all
- [ ] NOT TESTED

**B2. Is the device "readable" in the app (visible on the list, the app fetches NodeInfo - name, position, battery, telemetry)?**

- [ ] YES [ ] PARTIALLY [ ] NO [ ] NOT TESTED

**B3. Does the USB/serial connection work (app, `meshtastic` CLI, logging)?**

- [ ] YES [ ] PARTIALLY [ ] NO [ ] NOT TESTED

---

## 4. Baseline test - network WITHOUT this experimental firmware

_This rules out problems originating in the rest of the network rather than in the tested firmware. If you can, power off the other Adaptive Relay nodes for this test._

**C1. Does the device receive text messages from other nodes that do NOT run this experimental firmware (stock transmitters)?**

- [ ] YES - all of them / all noticed in time
- [ ] PARTIALLY - some messages don't arrive (estimate %: \_\_\_)
- [ ] NO - no reception
- [ ] NOT TESTED (the whole network runs AR firmware)

**C2. Do you see other nodes in the app/device list (NodeDB fills up, SNR/RSSI look realistic)?**

- [ ] YES [ ] PARTIALLY [ ] NO [ ] NOT TESTED

**C3. Do positions and/or telemetry arrive from stock nodes?**

- [ ] YES [ ] PARTIALLY [ ] NO [ ] NOT TESTED

---

## 5. Two-way communication (Adaptive Relay firmware)

**D1. Does the device send text messages (from the phone to other nodes)?**

- [ ] YES [ ] PARTIALLY [ ] NO [ ] NOT TESTED

**D2. Does the device receive text messages (including from nodes running the Adaptive Relay firmware)?**

- [ ] YES [ ] PARTIALLY [ ] NO [ ] NOT TESTED

**D3. Do delivery confirmations work (the app shows "delivered/read" status - ACKs)?**

- [ ] YES [ ] PARTIALLY [ ] NO [ ] NOT TESTED

**D4. Round-trip test: did you send a message and receive a reply?**

- [ ] YES [ ] NO [ ] NOT TESTED

---

## 6. Adaptive Relay layer (router role and `-log` variant)

**E1. Does the device rebroadcast packets (neighbors/distant nodes receive messages through this node)?**

- [ ] YES [ ] PARTIALLY [ ] NO [ ] DON'T KNOW / COULD NOT CHECK

**E2. (`-log` variant only) Does the serial log show the `AR: Adaptive Relay N3 diagnostic build …` banner and `AR-N3`/`AR-DUPE`/`AR-SHEP`/`AR-CB` lines?**

- [ ] YES [ ] PARTIALLY (some line types missing) [ ] NO [ ] NOT TESTED

**E3. (`-log` variant only) Does the `S:B:…` line point to the `StrazPrzyszlosci/MESHTASTIC_ADAPTIVE_RELAY_firmware` repo?**

- [ ] YES [ ] NO [ ] NOT TESTED

**E4. (`-log` variant only) Apart from the `AR-`/`S:B:` lines, is the log actually "quiet" (few telemetry/screen/NodeDB entries)?**

- [ ] YES [ ] NO (too much noise - examples?: \_\_\_)

---

## 7. Stability

**F1. Does the device run without unplanned restarts?**

- [ ] YES - 0 restarts during the test
- [ ] PARTIALLY - occasional reboots (how many?: \_\_\_)
- [ ] NO - regular freezes/restarts
- [ ] DON'T KNOW

**F2. Does the device come back to work after a long idle period/overnight (without a manual restart)?**

- [ ] YES [ ] NO [ ] NOT TESTED

**F3. Battery drain compared to the official firmware (if battery powered):**

- [ ] NO DIFFERENCE [ ] WORSE [ ] BETTER [ ] NOT TESTED

---

## 8. Observations (free form)

- Anomalies, errors, unusual behavior: \_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_
- \_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_
- \_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_

**Attachments (highly welcome):**

- [ ] serial log excerpt with `AR-`/`S:B:` lines (for the `-log` variant:
      `grep -E "AR-|S:B:"` over the serial capture)
- [ ] app screenshot with the node list
- [ ] configuration export (without keys!)

---

_By sharing test results you acknowledge that the firmware is experimental
and is not affiliated with or endorsed by the Meshtastic project. Do not
flash it on devices your safety depends on._
