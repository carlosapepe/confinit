library(plotly)
library(plyr)
library(ggplot2)

# Lab path
namefile37="/home/asbatista/Dropbox/Mestrado/Orientação/Simulação/Graphs/Ver2/890s/traces/stealth_global_Traces/Global_Coi_Trace_192.168.1.38.txt"
namefile52="/home/asbatista/Dropbox/Mestrado/Orientação/Simulação/Graphs/Ver2/890s/traces/stealth_global_Traces/Global_Coi_Trace_192.168.1.53.txt"
namefile70="/home/asbatista/Dropbox/Mestrado/Orientação/Simulação/Graphs/Ver2/890s/traces/stealth_global_Traces/Global_Coi_Trace_192.168.1.71.txt"
nameMeanFile="/home/asbatista/Dropbox/Mestrado/Orientação/Simulação/Graphs/Ver2/890s/performance/mean_cois_6_p.png"
#nameMeanFile="/home/asbatista/Dropbox/Mestrado/Orientação/Simulação/Graphs/Ver2/890s/performance/mean_cois_6_e.png"


# Home path
# namefile37="/Users/agnaldobatista/Dropbox/Mestrado/Orientação/Simulação/Graphs/Ver2/890s/traces/stealth_global_Traces/Global_Coi_Trace_192.168.1.38.txt"
# namefile52="/Users/agnaldobatista/Dropbox/Mestrado/Orientação/Simulação/Graphs/Ver2/890s/traces/stealth_global_Traces/Global_Coi_Trace_192.168.1.53.txt"
# namefile70="/Users/agnaldobatista/Dropbox/Mestrado/Orientação/Simulação/Graphs/Ver2/890s/traces/stealth_global_Traces/Global_Coi_Trace_192.168.1.71.txt"
# nameMeanFile="/Users/agnaldobatista/Dropbox/Mestrado/Orientação/Simulação/Graphs/Ver2/890s/Performance/meanCoIs.pdf"

font_eixo <- list(
  #family = "Arial",
  family = "Liberation Sans",
  #family = "Courier New, monospace",
  size = 22,
  color = "black")

font_axis <- list(
  #family = "Arial",
  family = "Liberation Sans",
  #family = "Courier New, monospace",
  size = 18,
  color = "black")

legenda <- list(
  font = list(
    #family = "Arial",
    family = "Liberation Sans",
    #family = "sans-serif",
    size = 20,
    color = "black"),
    x=0.05,
    y=0.99)#,
  #orientation = 'h')
eixo_x_1 <- list(
  title = "Nós",
  #title = "Nodes",
  tick0 = 0,
  tickwidth = 2,
  #dtick = 100,
  tickfont = font_axis,
  titlefont = font_eixo,
  mirror=TRUE,
  ticks='outside',
  showline=TRUE,
  gridcolor = toRGB("lightgray"),
  gridwidth = 1,
  showgrid = TRUE)
eixo_y_1 <- list(
  title = "Número de Comunidades",
  #title = "Number of Communities",  
  #dtick = 100,
  tickwidth = 2,
  range=c(0, 30),
  showline=TRUE,
  tick0 = 0,
  tickfont = font_axis,
  titlefont = font_eixo,
  mirror=TRUE,
  ticks='outside',
  showline=TRUE,
  gridcolor = toRGB("lightgray"),
  gridwidth = 1,
  showgrid = TRUE)

title_font <- list(
  #family = "Arial",
  family = "Liberation Sans",
  size = 22,
  color = "black")

data37 <- read.delim(namefile37, header = TRUE, sep="\t", quote="", dec = ".", fill = FALSE, comment.char = "")
data52 <- read.delim(namefile52, header = TRUE, sep="\t", quote="", dec = ".", fill = FALSE, comment.char = "")
data70 <- read.delim(namefile70, header = TRUE, sep="\t", quote="", dec = ".", fill = FALSE, comment.char = "")

data37_count <- c(1:length(data37$date)) # Contador do eixo x p/ plot 
data37_data <- data.frame(data37_count, data37$nCoiH) # Merge dos dados p/ plot

# GEt mean from communities columns
health37_mean = mean(data37$nCoiH) 
# music37_mean = mean(data37$nCoiC) 
# books37_mean = mean(data37$nCoiB) 
# tourism37_mean = mean(data37$nCoiT) 
# movies37_mean = mean(data37$nCoiM)

health52_mean = mean(data52$nCoiH) 
# music52_mean = mean(data52$nCoiC) 
# books52_mean = mean(data52$nCoiB) 
# tourism52_mean = mean(data52$nCoiT) 
# movies52_mean = mean(data52$nCoiM)

health70_mean = mean(data70$nCoiH) 
# music70_mean = mean(data70$nCoiC) 
# books70_mean = mean(data70$nCoiB) 
# tourism70_mean = mean(data70$nCoiT) 
# movies70_mean = mean(data70$nCoiM)

#Define each data sequence to be put in chart
health_data = c(health37_mean, health52_mean, health70_mean)
# music_data = c(music37_mean, music52_mean, music70_mean)
# books_data = c(books37_mean, books52_mean, books70_mean)
# tourism_data = c(tourism37_mean, tourism52_mean, tourism70_mean)
# movies_data = c(movies37_mean, movies52_mean, movies70_mean)

p <- plot_ly(x = c("37", "52", "70"), y = health_data,
             width=500, height=500, # Chart aspect ratio
             name = "Comunidades<br>de Saúde", type = "bar") %>%
             #name = "Health Communities", type = "bar") %>%
  layout(xaxis = eixo_x_1, yaxis = eixo_y_1, 
         title = "(a) Número médio de comunidades", # chart title
         #title = "Average number of communities", # chart title
         titlefont = title_font,
         showlegend = TRUE, legend = legenda,
         bargap=0.7)

p
export(p, file = nameMeanFile)