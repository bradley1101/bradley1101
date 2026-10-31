# MQTT topics — Smart Home

Broker: **EMQX Cloud** (TLS, port 8883). Every node publishes under `home/<node>/…` and subscribes to `home/<node>/+/set`.

Each node keeps a retained `home/<node>/status` = `online`, replaced by `offline` (Last Will) if it disconnects.

## Entrance — `home/entrance/…`

| Topic | Direction | Payload | Retained |
| --- | --- | --- | --- |
| `access` | node → broker | `{"uid":"A1B2C3D4","granted":true}` | no |
| `door/state` | node → broker | `open` / `closed` | yes |
| `visitor` | node → broker | `present` / `absent` | no |
| `door/set` | broker → node | `open` / `close` | — |

A valid badge opens the door for 5 s. `door/set open` keeps it open until `close`.

## Living room — `home/living/…`

| Topic | Direction | Payload | Retained |
| --- | --- | --- | --- |
| `motion` | node → broker | `1` / `0` | no |
| `light/state` | node → broker | `on` / `off` | yes |
| `light/mode` | node → broker | `auto` / `manual` | yes |
| `light/set` | broker → node | `on` / `off` / `auto` | — |

In `auto` mode the light follows the PIR and stays on 10 s after the last motion. `on` / `off` switch to manual mode.

## Kitchen — `home/kitchen/…`

| Topic | Direction | Payload | Retained |
| --- | --- | --- | --- |
| `climate` | node → broker | `{"t":28.0,"h":61.0}` (every 5 s) | no |
| `alert` | node → broker | `overheat` / `normal` | yes |
| `window/state` | node → broker | `open` / `closed` | yes |
| `window/mode` | node → broker | `auto` / `manual` | yes |
| `window/set` | broker → node | `open` / `close` / `auto` | — |

Above 35 °C the alert LED blinks and, in `auto` mode, the window opens. Back to normal below 33 °C (hysteresis).

## Testing without a dashboard

With [MQTTX](https://mqttx.app/) (or the EMQX Cloud web client), connect to the deployment and:

```text
subscribe  home/#
publish    home/entrance/door/set    open
publish    home/living/light/set     on
publish    home/kitchen/window/set   auto
```
