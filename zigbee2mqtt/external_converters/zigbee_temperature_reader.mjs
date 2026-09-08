// External Converter fuer Zigbee2MQTT: ZigbeeTemperatureReader (ESP32-C6)
//
// Zwei Endpoints (10 = Vorlauf, 11 = Ruecklauf) melden je einen eigenen
// Temperature-Measurement-Cluster. Zigbee2MQTT verarbeitet bei gleichartigen
// Clustern auf mehreren Endpoints standardmaessig nur den ersten Treffer
// (bekanntes Problem: https://github.com/Koenkk/zigbee2mqtt/issues/31172).
// Der klassische fromZigbee/exposes-Ansatz mit endpoint()+meta.multiEndpoint
// (wie bei mehrkanaligen Schaltern) umgeht das zuverlaessig.
import {temperature} from "zigbee-herdsman-converters/converters/fromZigbee";
import {presets} from "zigbee-herdsman-converters/lib/exposes";

export default {
    // Muss mit dem ModelIdentifier des Basic-Clusters auf dem niedrigsten
    // Endpoint (hier: Endpoint 10, Vorlauf) uebereinstimmen. Siehe
    // setManufacturerAndModel() in src/main.cpp. Falls Zigbee2MQTT das
    // Geraet weiterhin als "nicht unterstuetzt" meldet, den tatsaechlich
    // gemeldeten Wert aus dem Z2M-Log/Frontend uebernehmen.
    zigbeeModel: ["TempReader-Vorlauf"],
    model: "ZigbeeTemperatureReader",
    vendor: "DIY",
    description: "Vor-/Ruecklauftemperatur Heizkoerper (ESP32-C6)",
    fromZigbee: [temperature],
    toZigbee: [],
    exposes: [
        presets.temperature().withEndpoint("vorlauf").withHomeAssistant({name: "Vorlauf"}),
        presets.temperature().withEndpoint("ruecklauf").withHomeAssistant({name: "Ruecklauf"}),
    ],
    endpoint: (device) => {
        return {vorlauf: 10, ruecklauf: 11};
    },
    meta: {multiEndpoint: true},
};
