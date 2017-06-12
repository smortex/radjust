FILESIZE = Transform(/(\d+) ?([KMG])?(i)?B/) do |size, unit, si|
  multiplier = if si == 'i'
                 1024
               else
                 1000
               end

  power = [nil, 'K', 'M', 'G'].index(unit)

  size.to_i * multiplier**power
end
