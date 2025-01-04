import streamlit as st
import sqlite3
import pandas as pd
import time

# Global variables definitions
if 'patient_saved' not in st.session_state:
    st.session_state.patient_saved = False
if "user_email" not in st.session_state:
    st.session_state.user_email = None
if 'logged_in' not in st.session_state:
    st.session_state.logged_in = False
if 'codigo' not in st.session_state:
    st.session_state.codigo = None

USER_DATA = {
    "valu": "valu",
    "sofi": "sofi",
}


def login_page() -> None:
    st.image(r'./bonemarrow_background.jpeg', use_column_width=True)
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
  


def main():
    if st.session_state.logged_in:
        menu_page()

    else:
        login_page()

main()