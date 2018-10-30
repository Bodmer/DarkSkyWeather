/*

    [units] should be one of the following:

    auto: automatically select units based on geographic location
    ca: same as si, except that windSpeed and windGust are in kilometers per hour
    uk2: same as si, except that nearestStormDistance and visibility are in miles, and windSpeed and windGust in miles per hour
    us: Imperial units (the default)
    si: SI units


    SI units used:

    summary: Any summaries containing temperature or snow accumulation units will have
             their values in degrees Celsius or in centimeters (respectively).
    nearestStormDistance: Kilometers.
    precipIntensity: Millimeters per hour.
    precipIntensityMax: Millimeters per hour.
    precipAccumulation: Centimeters.
    temperature: Degrees Celsius.
    temperatureMin: Degrees Celsius.
    temperatureMax: Degrees Celsius.
    apparentTemperature: Degrees Celsius.
    dewPoint: Degrees Celsius.
    windSpeed: Meters per second.
    windGust: Meters per second.
    pressure: Hectopascals.
    visibility: Kilometers.

    moon phase is x 100 0-100%

    lang=[language] optional

    Return summary properties in the desired language.
    (Note that units in the summary will be set according to the units parameter,
    so be sure to set both parameters appropriately.) language may be:

    ar: Arabic
    az: Azerbaijani
    be: Belarusian
    bg: Bulgarian
    bs: Bosnian
    ca: Catalan
    cs: Czech
    da: Danish
    de: German
    el: Greek
    en: English (which is the default)
    es: Spanish
    et: Estonian
    fi: Finnish
    fr: French
    he: Hebrew
    hr: Croatian
    hu: Hungarian
    id: Indonesian
    is: Icelandic
    it: Italian
    ja: Japanese
    ka: Georgian
    ko: Korean
    kw: Cornish
    lv: Latvian
    nb: Norwegian Bokmål
    nl: Dutch
    no: Norwegian Bokmål (alias for nb)
    pl: Polish
    pt: Portuguese
    ro: Romanian
    ru: Russian
    sk: Slovak
    sl: Slovenian
    sr: Serbian
    sv: Swedish
    tet: Tetum
    tr: Turkish
    uk: Ukrainian
    x-pig-latin: Igpay Atinlay
    zh: simplified Chinese
    zh-tw: traditional Chinese

*/
