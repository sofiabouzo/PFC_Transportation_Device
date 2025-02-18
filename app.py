import streamlit as st
import pandas as pd
import time
from io import StringIO

# Global variables definitions
if 'patient_saved' not in st.session_state:
    st.session_state.patient_saved = False
if "user_email" not in st.session_state:
    st.session_state.user_email = None
if 'logged_in' not in st.session_state:
    st.session_state.logged_in = False
if 'codigo' not in st.session_state:
    st.session_state.codigo = None
if 'buscar' not in st.session_state:
    st.session_state.buscar= False
if 'graficar' not in st.session_state:
    st.session_state.graficar= False

USER_DATA = {
    "valu": "valu",
    "sofi": "sofi",
}


def login_page() -> None:
    st.image(r'./bonemarrow_background.jpeg', use_container_width=True)
    st.title("Inicio de Sesión")
    email = st.text_input("Email")
    password = st.text_input("Contraseña", type="password")
    if st.button("Iniciar Sesión"):
        if email in USER_DATA and USER_DATA[email] == password:
            st.session_state.logged_in = True
            st.session_state.user_email = email
            st.success("¡Inicio de sesión exitoso!")
            time.sleep(2)
            st.rerun()
        else:
            st.error("Email o contraseña incorrectos")


def menu_page():


    st.sidebar.write(f"**Bienvenido**: {st.session_state.user_email}")
    if st.sidebar.button("Cerrar Sesión"):
        st.session_state.logged_in = False
        st.rerun()

    with st.sidebar:
        st.markdown('<h1 style="color:#a01545;">🚑 Control Center</h1>', unsafe_allow_html=True)
        st.subheader("Ingrese el *tracking code* del traslado:")
        st.session_state.codigo= st.text_input("Código de seguimiento")
        if st.sidebar.button("Buscar"):
            st.session_state.buscar= True
        st.markdown('<h1 style="color:#a01545;">📈 Análisis tiempo temp</h1>', unsafe_allow_html=True)
        uploaded_file= st.sidebar.file_uploader("seleccione el archivo que quiera graficar:", type = ["csv"])
        if st.sidebar.button("Graficar"):
            st.session_state.graficar= True

    st.header("Ingrese el código para continuar")
    if st.session_state.buscar:
        st.subheader("Estado de transporte", anchor= "status", divider= 'grey')
        st.write("El dispositivo se encuentra: ....")
        st.subheader("Temperatura", anchor= "temperature", divider= 'grey')
        st.write("La temperatura actual es:")
    
    if st.session_state.graficar:
        st.header(f"Grafico del documento {uploaded_file.name}" )
        
        df = pd.read_csv(uploaded_file, sep=",", encoding="utf-8")
        st.dataframe(df)
        st.line_chart(df,x='Tiempo', y="Temperatura", x_label= "Tiempo [s]", y_label= "Temperatura [ºC]", color="#a1677b")

def main():
    if st.session_state.logged_in:
        menu_page()
 
    else:
        login_page()

main()