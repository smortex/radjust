FILESIZE = Transform(/(\d+) ?([KMG])?(i)?B/) do |size, unit, si|
  multiplier = if si == 'i'
                 1000
               else
                 1024
               end

  power = [nil, 'K', 'M', 'G'].index(unit)

  size.to_i * multiplier**power
end
