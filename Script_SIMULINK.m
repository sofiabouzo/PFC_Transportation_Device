%% Este script ejecuta y crea las variables necesarias para correr la simulacion de SIMULINK
% PROYECTO FINAL VALEN Y SOFI
clear; clc;
data = readmatrix('registros/datos_18.02_sincarga.csv');
tiempo = data(1:89,1);
temperatura = data(1:89,3);
tiempo = [0; tiempo]; 
temperatura = [21.04; temperatura];
curva_experimental.time = tiempo;
curva_experimental.signals.values = temperatura;
curva_experimental.signals.dimensions = 1;
input=ones(size(temperatura))*11;

curva_input.time = tiempo;
curva_input.signals.values = input;
curva_input.signals.dimensions = 1;
time = (0:30:2670)';
load('Transferenca_1.mat');

temp_encero = temperatura-21.04*ones(size(temperatura));
temp_encero=(temp_encero*-1); %Invertimos la curva para que comience en cero

load('Transferenca_2.mat');
load('PID_Invertido.mat');
