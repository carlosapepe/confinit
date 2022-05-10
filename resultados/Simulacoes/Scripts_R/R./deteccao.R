  library(plotly)
  library(plyr)
  library(ggplot2)
  library(ggplot)
  
  # Lab path
  namefile50_10="/home/carlos/Documentos/Mestrado2019/Sbseg2019/Simulacoes/Dados/50nos_10.txt"
  namefile75_10="/home/carlos/Documentos/Mestrado2019/Sbseg2019/Simulacoes/Dados/75nos_10.txt"
  namefile100_10="/home/carlos/Documentos/Mestrado2019/Sbseg2019/Simulacoes/Dados/100nos_10.txt"
  namefile50_20="/home/carlos/Documentos/Mestrado2019/Sbseg2019/Simulacoes/Dados/50nos_20.txt"
  namefile75_20="/home/carlos/Documentos/Mestrado2019/Sbseg2019/Simulacoes/Dados/75nos_20.txt"
  namefile100_20="/home/carlos/Documentos/Mestrado2019/Sbseg2019/Simulacoes/Dados/100nos_20.txt"
  namefile50_30="/home/carlos/Documentos/Mestrado2019/Sbseg2019/Simulacoes/Dados/50nos_30.txt"
  namefile75_30="/home/carlos/Documentos/Mestrado2019/Sbseg2019/Simulacoes/Dados/75nos_30.txt"
  namefile100_30="/home/carlos/Documentos/Mestrado2019/Sbseg2019/Simulacoes/Dados/100nos_30.txt"
  
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
      x=0.7,
      y=0.100)#,
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
    dtick = (10),
    tickwidth = 2,
    range=c(0, 100),
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
  
  #10% de atacantes na rede
  data50_10 <- read.delim(namefile50_10, header = TRUE, sep="\t", quote="", dec = ".", fill = FALSE, comment.char = "")
  data75_10 <- read.delim(namefile75_10, header = TRUE, sep="\t", quote="", dec = ".", fill = FALSE, comment.char = "")
  data100_10 <- read.delim(namefile100_10, header = TRUE, sep="\t", quote="", dec = ".", fill = FALSE, comment.char = "")
  
  #Leitura da primeiro coluna 50 nós
  data50_10_count <- c(1:length(data50_10$S)) # Contador do eixo x p/ plot 
  data50_data_10 <- data.frame(data50_10_count, data50_10$S) # Merge dos dados p/ plot
  
  #leitura da segunda coluna 50 nós
  data50_10c_count <- c(1:length(data50_10$Na)) # Contador do eixo x p/ plot 
  data50c_10_data <- data.frame(data50c_count, data50_10$Na) # Merge dos dados p/ 
  
  #Leitura da primeiro coluna 75 nós
  data75_10_count <- c(1:length(data75_10$S)) # Contador do eixo x p/ plot 
  data75_10_data <- data.frame(data75_10_count, data75_10$S) # Merge dos dados p/ plot
  
  #Leitura da segunda coluna 75 nós
  data75c_10_count <- c(1:length(data75_10$Na)) # Contador do eixo x p/ plot 
  data75c_10_data <- data.frame(data75c_count, data75_10$Na) # Merge dos dados p/ plot
  
  #Leitura da primeiro coluna 100 nós 
  data100_10_count <- c(1:length(data100_10$S)) # Contador do eixo x p/ plot 
  data100_10_data <- data.frame(data100_10_count, data100_10$S) # Merge dos dados p/ plot
  
  #Leitura da primeiro coluna 100 nós 
  data100c_10_count <- c(1:length(data100$Na)) # Contador do eixo x p/ plot 
  data100c_10_data <- data.frame(data100c_10_count, data100_10$Na) # Merge dos dados p/ plot
  
  # # GEt mean from communities columns
  no50_10_mean = mean(data50_10$S)
  no50_10_sum = sum(data50_10$S)
  no50c_10_mean = mean(data50_10$Na)
  no50c_10_sum = sum(data50_10$Na)
  
  por50_10 <- (no50c_10_sum/no50_10_sum)*100
  
  no75_10_mean = mean(data75_10$S)
  no75_10_sum = sum(data75_10$S)
  no75c_10_mean = mean(data75_10$Na)
  no75c_10_sum = sum(data75_10$Na)
  
  
  por75_10 <- (no75c_10_sum/no75_10_sum)*100
  
  no100_10_mean = mean(data100_10$S)
  no100_10_sum = sum(data100_10$S)
  no100c_10_mean = mean(data100_10$Na)
  no100c_10_sum = sum(data100_10$Na)
  
  por100_10 <- (no100c_10_sum/no100_10_sum)*100
  
  #MÉDIA DE DETECÇÃO
  media_10 = (por50_10+por75_10+por100_10)/3
  
  #20% de atacantes na rede 
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
  
  por100_20 <- (no100c_20_sum/no100_20_sum)*100
  
  media_20 = (por50_20+por75_20+por100_20)/3
  
  #30% de atacantes na rede 
  data50_30 <- read.delim(namefile50_30, header = TRUE, sep="\t", quote="", dec = ".", fill = FALSE, comment.char = "")
  data75_30 <- read.delim(namefile75_30, header = TRUE, sep="\t", quote="", dec = ".", fill = FALSE, comment.char = "")
  data100_30 <- read.delim(namefile100_30, header = TRUE, sep="\t", quote="", dec = ".", fill = FALSE, comment.char = "")
  
  #Leitura da primeiro coluna 50 nós
  data50_30_count <- c(1:length(data50_30$S)) # Contador do eixo x p/ plot 
  data50_data_30 <- data.frame(data50_30_count, data50_30$S) # Merge dos dados p/ plot
  
  #leitura da segunda coluna 50 nós
  data50_30c_count <- c(1:length(data50_30$Na)) # Contador do eixo x p/ plot 
  data50c_30_data <- data.frame(data50c_count, data50_30$Na) # Merge dos dados p/ 
  
  #Leitura da primeiro coluna 75 nós
  data75_30_count <- c(1:length(data75_30$S)) # Contador do eixo x p/ plot 
  data75_30_data <- data.frame(data75_30_count, data75_30$S) # Merge dos dados p/ plot
  
  #Leitura da segunda coluna 75 nós
  data75c_30_count <- c(1:length(data75_30$Na)) # Contador do eixo x p/ plot 
  data75c_30_data <- data.frame(data75c_count, data75_30$Na) # Merge dos dados p/ plot
  
  #Leitura da primeiro coluna 100 nós 
  data100_30_count <- c(1:length(data100_30$S)) # Contador do eixo x p/ plot 
  data100_30_data <- data.frame(data100_30_count, data100_30$S) # Merge dos dados p/ plot
  
  #Leitura da primeiro coluna 100 nós 
  data100c_30_count <- c(1:length(data100$Na)) # Contador do eixo x p/ plot 
  data100c_30_data <- data.frame(data100c_30_count, data100_30$Na) # Merge dos dados p/ plot
  
  # # GEt mean from communities columns
  no50_30_mean = mean(data50_30$S)
  no50_30_sum = sum(data50_30$S)
  no50c_30_mean = mean(data50_30$Na)
  no50c_30_sum = sum(data50_30$Na)
  
  por50_30 <- (no50c_30_sum/no50_30_sum)*100
  
  no75_30_mean = mean(data75_30$S)
  no75_30_sum = sum(data75_30$S)
  no75c_30_mean = mean(data75_30$Na)
  no75c_30_sum = sum(data75_30$Na)
  
  
  por75_30 <- (no75c_30_sum/no75_30_sum)*100
  
  no100_30_mean = mean(data100_30$S)
  no100_30_sum = sum(data100_30$S)
  no100c_30_mean = mean(data100_30$Na)
  no100c_30_sum = sum(data100_30$Na)
  
  por100_30 <- (no100c_30_sum/no100_30_sum)*100
  
  media_30 = (por50_30+por75_30+por100_30)/3
   
  # # #Define each data sequence to be put in chart
  detec_data_1 = c(por50_10, por75_10, por100_10)
  detec_data_2 = c(por50_20, por75_20, por100_20)
  detec_data_3 = c(por50_30, por75_20, por100_30)
  #detec_data = c(por50, por75) #por100)
  
  media_final = (media_10+media_20+media_30)/3
  
  # # # music_data = c(music37_mean, music52_mean, music70_mean)
  # # # books_data = c(books37_mean, books52_mean, books70_mean)
  # # # tourism_data = c(tourism37_mean, tourism52_mean, tourism70_mean)
  # # # movies_data = c(movies37_mean, movies52_mean, movies70_mean)
  # # 
  
  
  rede <- c("50", "75", "99")
  ataq_10 <- c(5,4,3.5)
  ataq_20 <- c(4,3,4.2)
  ataq_30 <- c(5.1,3.0,6)
  data <- data.frame(rede, ataq_10, ataq_20, ataq_30)
  
  banco.dados <- data.frame(Fudeo = factor(c(rede), 
                                           levels=c(ataq_10)), total = c(rede))
  
  ggplot(data=banco.dados, aes(x=Fudeo, y=rede)) + geom_bar(stat="identity")
  
   
  
  
   
   # p <- plot_ly(data, x =~factor(rede), y = ~ataq_10,
   #             width=500, height=500,
   #             type = 'bar', marker = list(color = 'rgb(55, 83, 109)'),
   #             #marker = list(colors = colors),
   #             name = '10%_ataque') %>%
   #             add_trace(y = ~ataq_20, name = '20%_ataque', 
   #             marker = list(color = 'rgba(222,45,38,0.8)')) %>%
   #             add_trace(y = ~ataq_30, name = '30%_ataque', 
   #             marker = list(color = 'rgba(204,204,204,1)')) %>%
   #             
   #    layout(xaxis = eixo_x_1, yaxis = eixo_y_1, 
   #           title = "(a) Taxa média de Ataque IDF ", # chart title
   #           #title = "Average number of communities", # chart title
   #           #titlefont = title_font,
   #           showlegend = TRUE, legend = legenda,
   #           bargap=0.4)
   #  
   # p
   # #export(p, file = nameMeanFile)
   # 
   # 