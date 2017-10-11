ParameterType(
  name: 'filesize',
  regexp: /(\d+) ([KMG]?)(i?)B/,
  transformer: lambda do |size, unit, si|
    multiplier = if si == 'i'
                   1024
                 else
                   1000
                 end

    power = ['', 'K', 'M', 'G'].index(unit)

    size.to_i * multiplier**power
  end
)
