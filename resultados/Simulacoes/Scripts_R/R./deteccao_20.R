library(plotly)
library(plyr)
library(ggplot2)

Sys.setenv("PATH" = paste(Sys.getenv("PATH"), "/home/carlos/anaconda3/bin/", sep = .Platform$path.sep))

# Lab path
namefile50_20="/home/carlos/Documentos/Mestrado2019/Sbseg2019/Simulacoes/Dados/50nos_20.txt"
namefile75_20="/home/carlos/Documentos/Mestrado2019/Sbseg2019/Simulacoes/Dados/75nos_20.txt"
namefile100_20="/home/carlos/Documentos/Mestrado2019/Sbseg2019/Simulacoes/Dados/100nos_20.txt"
#namefile75="/home/asbatista/Dropbox/Mestrado/Orientação/Simulação/Graphs/Ver2/890s/traces/stealth_global_Traces/Global_Coi_Trace_192.168.1.53.txt"
#namefile10="/home/asbatista/Dropbox/Mestrado/Orientação/Simulação/Graphs/Ver2/890s/traces/stealth_global_Traces/Global_Coi_Trace_192.168.1.71.txt"
#nameMeanFile="/home/asbatista/Dropbox/Mestrado/Orientação/Simulação/Graphs/Ver2/890s/performance/mean_cois_6_p.png"
#nameMeanFile="/home/asbatista/Dropbox/Mestrado/Orientação/Simulação/Graphs/Ver2/890s/performance/mean_cois_6_e.png"
nameMeanFile="/home/carlos/Documentos/Mestrado2019/Sbseg2019/Simulacoes/Dados/imagem_1.png"

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
    size = 10,
    color = "black"),
  x=0.70,
  y=0.99)#,
#orientation = 'h')
eixo_x_1 <- list(
  title = "Número de Nós na Rede",
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
  showgrid = FALSE)
eixo_y_1 <- list(
  title = "Taxa de Detecção (%)",
  #title = "Number of Communities",  
  dtick = (5),
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

data50_20 <- read.delim(namefile50_20, header = TRUE, sep="\t", quote="", dec = ".", fill = FALSE, comment.char = "")
data75_20 <- read.delim(namefile75_20, header = TRUE, sep="\t", quote="", dec = ".", fill = FALSE, comment.char = "")
data100_20 <- read.delim(namefile100_20, header = TRUE, sep="\t", quote="", dec = ".", fill = FALSE, comment.char = "")

#Leitura da primeiro coluna 50 nós
data50_20_count <- c(1:length(data50_20$S)) # Contador do eixo x p/ plot 
data50_data_20 <- data.frame(data50_20_count, data50_20$S) # Merge dos dados p/ plot

#leitura da segunda coluna 50 nós
data50_20c_count <- c(1:length(data50_20$Na)) # Contador do eixo x p/ plot 
data50c_20_data <- data.frame(data50c_count, data50_20$Na) # Merge dos dados p/ 

#Leitura da primeiro coluna 75 nós
data75_20_count <- c(1:length(data75_20$S)) # Contador do eixo x p/ plot 
data75_20_data <- data.frame(data75_20_count, data75_20$S) # Merge dos dados p/ plot

#Leitura da segunda coluna 75 nós
data75c_20_count <- c(1:length(data75_20$Na)) # Contador do eixo x p/ plot 
data75c_20_data <- data.frame(data75c_count, data75_20$Na) # Merge dos dados p/ plot

#Leitura da primeiro coluna 100 nós 
data100_20_count <- c(1:length(data100_20$S)) # Contador do eixo x p/ plot 
data100_20_data <- data.frame(data100_20_count, data100_20$S) # Merge dos dados p/ plot

#Leitura da primeiro coluna 100 nós 
data100c_20_count <- c(1:length(data100$Na)) # Contador do eixo x p/ plot 
data100c_20_data <- data.frame(data100c_20_count, data100_20$Na) # Merge dos dados p/ plot

# # GEt mean from communities columns
no50_20_mean = mean(data50_20$S)
no50_20_sum = sum(data50_20$S)
no50c_20_mean = mean(data50_20$Na)
no50c_20_sum = sum(data50_20$Na)

por50_20 <- (no50c_20_sum/no50_20_sum)*100

no75_20_mean = mean(data75_20$S)
no75_20_sum = sum(data75_20$S)
no75c_20_mean = mean(data75_20$Na)
no75c_20_sum = sum(data75_20$Na)


por75_20 <- (no75c_20_sum/no75_20_sum)*100

no100_20_mean = mean(data100_20$S)
no100_20_sum = sum(data100_20$S)
no100c_20_mean = mean(data100_20$Na)
no100c_20_sum = sum(data100_20$Na)

por100_20 <- (no100c_sum/no100_sum)*100

# # #Define each data sequence to be put in chart
detec_data = c(por50_20, por75_20, por100_20)
#detec_data = c(por50, por75) #por100)


# rede <- c("50", "75", "100")
# ataq_20 <- c(detec_data)
# ataq_30 <- c(92, 95, 96)
# data <- data.frame(rede, ataq_20, ataq_30)
# 
# p <- plot_ly(data, x =~factor(rede), y = ~ataq_20, 
#              width=500, height=500,
#              type = 'bar', 
#              name = '20%_ataque') %>%
#   add_trace(y = ~ataq_30, name = '30%_ataque') %>%
#   
p <- plot_ly(
  x = c("50","75","99"), 
  #y = c(90,94,95),
  #y = c(detec_data),
  y = c(10,15,20),
  #marker = list(color = 'rgb(55, 83, 109)'), 
  #marker = list(color = 'rgba(204,204,204,1)'),
  width=500, height=500,  #Chart aspect ratio
  #name = "", 
  type = "bar") %>%
  #list(color = 'rgb(158,202,225)',
  #name = "Health Communities", type = "bar") %>%
  layout(xaxis = eixo_x_1, yaxis = eixo_y_1, 
         title = "(a) 20% de ataque IDF ",  #chart title,
         #title = "Average number of communities", # chart title
         #titlefont = title_font,
         #showlegend = TRUE, #legend = legenda,
         bargap=0.5)

#
p
orca(p, "detec_20.png")